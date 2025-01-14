/******************************************************************************
 * INCLUDES
 *****************************************************************************/

/* Standard C headers */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* XDCtools Header files */
#include <xdc/cfg/global.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Timestamp.h>
#include <xdc/runtime/Types.h>
#include <xdc/std.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Swi.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drv/gpio/GPIO.h>
#include <ti/drv/gpio/soc/GPIO_soc.h>
#include <ti/drv/gpio/test/led_blink/src/GPIO_board.h>

/* Board specific headers */
#include <ti/board/board.h>
#include <ti/csl/csl_clec.h>

/* User headers */
#include "QRS_Dat_in.h"
#include "Baseline_Wander_Coeffs.h"
#include "Anti_Aliasing_Coeffs.h"
#include "QRS_Enhance_Coeffs.h"

/******************************************************************************
 * DEFINES & MACROS
 *****************************************************************************/

#define SAMPLE_FREQ 80 /* sampling frequency in Hz */
#define BUFFER_SIZE 170 /* number of samples in the input signal */
#define TIMER_PERIOD 1000000 / SAMPLE_FREQ /* timer period in microseconds */

/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/******************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

volatile float g_ecg_buffer[BUFFER_SIZE]; /* circular buffer for storing ECG samples */
volatile float g_ecg_filtered_buffer[BUFFER_SIZE]; /* buffer for fitlered ECG samples */
volatile uint16_t g_buffer_index = 0; /* current index pos in buffer */
volatile uint16_t g_process_index = 0; /* processing index for filtered buffer */

/******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES
 *****************************************************************************/

/*!
 * @brief Write to circular buffer
 *
 * Writes a value (in this case samples) to the specified circular buffer and
 * updates index. resets index to 0 when reaching buffer size.
 *
 * @param buffer -  pointer to the circular buffer
 * @param index - pointer to the current write index
 * @param value - value to write into the buffer
 * @param size - size of the buffer
 */
static void buffer_write(volatile float *buffer, volatile uint16_t *index, float value, uint16_t size);

static float buffer_read(volatile float *buffer, uint16_t index, uint16_t size);
/*!
 * @brief IIR Biquad filter - Direct Form II
 * @param sample - most recent sample
 *
 * Direct Form II Implementation:
 * d[n] = x[n] - a1*d[n-1] - a2*d[n-2]    // intermediate state
 * y[n] = b0*d[n] + b1*d[n-1] + b2*d[n-2] // output
 *
 * where:
 * - d[n] represents intermediate states (delay elements)
 * - b[k] are numerator coefficients (feed-forward)
 * - a[k] are denominator coefficients (feedback)
 * - a0 is normalized to 1.0
 *
 * @return Filtered output sample with applied gain
 */
float iir_biquad_filter(const float (*a)[3], const float (*b)[3], float (*d)[2], int num_stages, float sample);

/*!
 * @brief Baseline wander removal high-pass filter
 *
 * Removes low-frequency baseline drift below 0.5Hz caused by:
 * - Patient breathing (0.15-0.3Hz)
 * - Body movements
 * - Poor electrode contact
 * - Electrode impedance changes
 * 
 * - Cutoff frequency: 0.5 Hz
 * 
 * @param sample The current input sample to filter
 * @return Filtered output sample
 */
float baseline_wander_filter(float sample);

/*!
 * @brief Anti-aliasing low-pass filter
 * 
 * This IIR filter removes high frequency components above Nyquist freq to prevent aliasing effects.
 * With a sampling rate of 80Hz, the Nyquist freq is 40Hz,
 * so any frequencies above this must be attenuated to prevent them from appearing
 * as false lower frequencies in the sampled signal.
 * 
 * - Cutoff frequency: 40 Hz
 * 
 * @param sample The current input sample to filter
 * @return Filtered output sample
 */
float anti_aliasing_filter(float sample);

/*!
 * @brief QRS complex enhancement band-pass filter
 *
 * Enhances the QRS complex while attenuating other ECG components.
 * The QRS complex contains frequencies mainly between 10-25Hz.
 * This filter:
 * - Amplifies the QRS complex frequency range
 * - Attenuates P and T waves (below 10Hz)
 * - Reduces high-frequency noise (above 25Hz)
 *
 * - Passband: 10-25 Hz
 * 
 * @param sample The current input sample to filter
 * @return Filtered output sample
 */
float qrs_enhance_filter(float sample);

/******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 *****************************************************************************/

/******************************************************************************
 * PRIVATE FUNCTION IMPLEMENTATIONS
 *****************************************************************************/

static void buffer_write(volatile float *buffer, volatile uint16_t *index, float value, uint16_t size)
{
	/* write value to current index */
	buffer[*index] = value;

	/* increment index */
	(*index)++;

	/* reset index if we reach the end */
	if (*index >= size)
	{
		*index = 0;
	}
}

static float buffer_read(volatile float *buffer, uint16_t index, uint16_t size)
{
  /* ensure index is inside bounds */
  index = index % size;

  /* return value at index */
  return buffer[index];
}

float iir_biquad_filter(const float (*b)[3], const float (*a)[3], float (*d)[2], int num_stages, float sample)
{
	/* init variables at the start of the fxn */
	float output_y = sample;
	float intermediate = 0.0f;
	int curr_stage = 0;

	/* process each stage in cascade */
	for (; curr_stage < num_stages; curr_stage++)
	{
		/* get intermidate value */
		/* calculate d[n] = x[n] - a1*d[n-1] - a2*d[n-2] */
		intermediate = output_y - (a[curr_stage][1] * d[curr_stage][0]) - (a[curr_stage][2] * d[curr_stage][1]);

		/* calculate output using b coefficients */
		/* calculate y[n] = b0*d[n] + b1*d[n-1] + b2*d[n-2] */
		output_y = b[curr_stage][0] * intermediate + (b[curr_stage][1] * d[curr_stage][0]) + (b[curr_stage][2] * d[curr_stage][1]);

		/* update state variables */
		d[curr_stage][1] = d[curr_stage][0]; /* d[n-2] = d[n-1] */
		d[curr_stage][0] = intermediate; /* d[n-1] = d[n] */
	}

	/* return filtered output */
	return output_y;
}

float baseline_wander_filter(float sample)
{
	static float d_baseline[BASELINE_FILTER_STAGES][2] = { { 0.0f } };

	return iir_biquad_filter(baseline_num, baseline_den, d_baseline, BASELINE_FILTER_STAGES, sample);
}

float anti_aliasing_filter(float sample)
{
	static float d_alias[BASELINE_FILTER_STAGES][2] = { { 0.0f } };

	return iir_biquad_filter(alias_num, alias_den, d_alias, ALIAS_FILTER_STAGES, sample);
}

float qrs_enhance_filter(float sample)
{
  static float d_qrs[BASELINE_FILTER_STAGES][2] = { { 0.0f } };

  return iir_biquad_filter(qrs_enhance_num, qrs_enhance_den, d_qrs, QRS_FILTER_STAGES, sample);
}

/*******************************************************************************
 * PUBLIC FUNCTION IMPLEMENTATIONS
 ******************************************************************************/

/*******************************************************************************
 * HWI/SWI FUNCTION IMPLEMENTATIONS
 ******************************************************************************/

/*!
 * @brief Timer64P0 (Timer ID 0) ISR for ECG sampling
 *
 * This ISR is triggred by Timer64P0 (32-bit mode) at 80 sampling freq.
 * the timer is configured to generate interrupts every 12.5ms.
 * Each interrupt triggers sample which is stored in a circular buffer.
 *
 * Timer configuration:
 * - Hardware: Timer64P0 (Timer A - ID 0)
 * - Mode: 32-bit unchained (chained - can be combined with timer B - ID 1 to 64 bit timer)
 * - Start Mode: starts automatically
 *
 * - Run Mode: periodic and continuous
 *
 * @note this function runs in interrupt context and should be kept as short as possible to prevent timing issues.
 */

Void ECG_Timer_ISR(Void)
{
  /* read the new sample from the QRS_IN data */
  float sample = buffer_read(QRS_IN, g_buffer_index, QRS_BUFFER_SIZE);

  Log_info2("New sample[%d]: %f", (IArg)g_buffer_index, (IArg)(sample * 1000));

  /* write into the cyclic buffer */
	buffer_write(g_ecg_buffer, &g_buffer_index, sample, BUFFER_SIZE);

  /* signal that new sample is ready */
  Semaphore_post(g_sample_ready_sem);
}

/******************************************************************************
 * TASK FUNCTION IMPLEMENTATIONS
 *****************************************************************************/

/*!
 * @brief ECG Preprocessing Task
 *
 * This task performs preprocessing operations on the ECG signal:
 * 1. Baseline Wander Removal using IIR filter
 * 2. Anti-Aliasing filter using IIR filter
 * 3. QRS Complex enhancement using IIR filter
 *
 * Operation:
 * - runs periodically to process new samples from ecg_buffer
 * - applies filters to remove noise
 * - stores filtered results in filtered_ecg_buffer
 * - maintains separate processing index to track progress
 *
 * @param arg0 task argument (unused)
 * @param arg1 task argument (unused)
 *
 * @note this task should be configured with lower priority than sampling to prevent timing issues.
 */
Void ECG_preprocess_Task(UArg arg0, UArg arg1)
{ /* infinite task loop */
	while (1)
	{
    /* wait for signal that new sample is ready */
    Semaphore_pend(g_sample_ready_sem, BIOS_WAIT_FOREVER);
    
    float input_sample = buffer_read(g_ecg_buffer, g_process_index, BUFFER_SIZE);
    Log_info2("Processing sample[%d]: %f", (IArg)g_process_index, (IArg)(input_sample * 1000));

    /* apply the Baseline Wander removal filter on the sample */
    float filtered_sample = baseline_wander_filter(input_sample);
    Log_info1("Filtered output: %f", (IArg)(filtered_sample * 1000));

    /* apply the Anti-Aliasing filter on the sample */
    //filtered_sample = anti_aliasing_filter(filtered_sample);
    
    /* apply the QRS enhance filter on the sample */
    //filtered_sample = qrs_enhance_filter(filtered_sample);

		/* write filtered sample to the buffer */
		buffer_write(g_ecg_filtered_buffer, &g_process_index, filtered_sample, BUFFER_SIZE);
	}
}

/******************************************************************************
 * MAIN FUNCTION
 *****************************************************************************/
int main(void)
{
	BIOS_start();
	return (0);
}
