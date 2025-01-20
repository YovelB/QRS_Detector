/******************************************************************************
 * INCLUDES
 *****************************************************************************/

/* Standard C headers */
#include <stddef.h> /* for size_t - for 64 bit machines - unsigned int 64 bit */

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/Log.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Swi.h>
#include <ti/sysbios/knl/Task.h>

/* User headers */
#include "QRS_Dat_in.h"
#include "config/config.h"
#include "buffers/buffer.h"
#include "filters/ecg_filters.h"
#include "feature_extract/qrs_detect.h"

/******************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/* global variable definitions - actual memory allocation happens here */
/* needs to be volatile - accessed by ISR and task concurrently */
volatile float g_ecg_buffer[BUFFER_SIZE]; /* circular buffer for storing ECG samples */
volatile float g_ecg_filtered_buffer[BUFFER_SIZE]; /* buffer for fitlered ECG samples */
volatile size_t g_buffer_index = 0; /* current index pos in buffer */
volatile size_t g_filtered_index = 0; /* processing index for filtered buffer */

/* Pan-Tompkins algorithm buffers and indices */
float g_derivative_buffer[BUFFER_SIZE]; /* buffer for derivative filter output */
size_t g_derivative_index = 0; /* index for derivative buffer */

float g_squared_buffer[BUFFER_SIZE]; /* buffer for squared signal output */
size_t g_squared_index = 0; /* index for squared buffer */

float g_mwi_buffer[MWI_WINDOW_SIZE]; /* buffer for moving window integration */
size_t g_mwi_index = 0; /* index for MWI buffer (wraps around MWI_WINDOW_SIZE) */

/* Buffer to store MWI output */
float g_mwi_output[BUFFER_SIZE]; /* buffer to store MWI results */
size_t g_mwi_output_index = 0; /* index for MWI output buffer */

/* init QRS detection parameters */
qrs_params_t g_qrs_params = { .signal_threshold = 0.0f, .noise_threshold = 0.0, .peak_value = 0.0f, .rr_interval = 0,
		.last_qrs_index = 0 };

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

	/* write into the cyclic buffer */
	buffer_write(g_ecg_buffer, &g_buffer_index, sample, BUFFER_SIZE);

	/* signal that new sample is ready */
	Semaphore_post(g_sample_ready_sem);
}

/******************************************************************************
 * TASK FUNCTION IMPLEMENTATIONS
 *****************************************************************************/

/*!
 * @brief ECG Signel Condition Task
 *
 * This task performs signel conditions operations on the ECG signal:
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
Void ECG_SignalConditionTask(UArg arg0, UArg arg1)
{
	/* infinite task loop */
	while (1)
	{
		/* wait for signal that new sample is ready */
		Semaphore_pend(g_sample_ready_sem, BIOS_WAIT_FOREVER);

		float input_sample = buffer_read(g_ecg_buffer, g_filtered_index, BUFFER_SIZE);
    Log_info1("Raw ECG: %d", (IArg)(input_sample * 1000));

		/* apply the Baseline Wander removal filter on the sample */
		float filtered_sample = baseline_wander_filter(input_sample);
    Log_info1("Baseline filtered: %d", (IArg)(filtered_sample * 1000));

		/* apply the LowFreq noise filter on the sample */
		filtered_sample = lowfreq_noise_filter(filtered_sample);
    Log_info1("LowFreq filtered: %d", (IArg)(filtered_sample * 1000));

		/* apply the Anti-Aliasing filter on the sample */
		filtered_sample = anti_aliasing_filter(filtered_sample);
    Log_info1("Anti alias filtered: %d", (IArg)(filtered_sample * 1000));

		/* apply the QRS enhance filter on the sample */
		filtered_sample = qrs_enhance_filter(filtered_sample);
    Log_info1("QRS enhanced: %d", (IArg)(filtered_sample * 1000));

		/* write filtered sample to the buffer */
		buffer_write(g_ecg_filtered_buffer, &g_filtered_index, filtered_sample, BUFFER_SIZE);

		/* signal that the filtered sample is ready */
		Semaphore_post(g_filtered_ready_sem);
	}
}

/*!
 * @brief QRS Complex Detection Task
 *
 * Implements real-time QRS detection using Pan-Tompkins algorithm stages:
 * 1. Differentiation
 * 2. Signal squaring
 * 3. Moving window integration
 * 4. Adaptive threshold detection
 *
 * Processing chain:
 * - Input: Filtered ECG samples from DSP preprocessing
 * - Output: QRS complex detection markers and timing
 * - Adaptive threshold adjustment for robust detection
 *
 * @param arg0 Task argument (unused)
 * @param arg1 Task argument (unused)
 */
Void ECG_FeatureDetectTask(UArg arg0, UArg arg1)
{
	/* the derivative method needs atleast 5 samples 2 future 2 past and 1 current sample */
	/* however the MWI_WINDOW_SIZE is 30 so we need to wait atleast for 30 samples */
	const size_t SAMPLES_NEEDED = 30;

	/* execute feature_extract every new sample */
	while (1)
	{
		/* wait for filtered sample from signal conditioning task */
		Semaphore_pend(g_filtered_ready_sem, BIOS_WAIT_FOREVER);

		/* Skip processing until we have enough samples */
		if (g_filtered_index < SAMPLES_NEEDED)
		{
			continue;
		}

		/* read latest filtered sample */
		float filtered_sample = buffer_read(g_ecg_filtered_buffer, g_squared_index, BUFFER_SIZE);

		/* step 1: derivative filter */
		float derivative_output = derivative_filter(g_ecg_filtered_buffer, g_derivative_index, filtered_sample);
		buffer_write(g_derivative_buffer, &g_derivative_index, derivative_output, BUFFER_SIZE);
		Log_info1("Derivative output: %f", (IArg)(derivative_output * 1000));

		/* step 2: squaring operation */
		float squared_output = square_signal(derivative_output);
		buffer_write(g_squared_buffer, &g_squared_index, squared_output, BUFFER_SIZE);
		Log_info1("Squared output: %f", (IArg)(squared_output * 1000));

		/* step 3: moving window integration */
		float mwi_output = moving_window_integrate(g_mwi_buffer, &g_mwi_index, squared_output);
		buffer_write(g_mwi_output, &g_mwi_output_index, mwi_output, BUFFER_SIZE);
		Log_info1("MWI output: %f", (IArg)(mwi_output * 1000));

		/* step 4: adaptive peak detection */
		if (detect_peaks(&g_qrs_params, mwi_output))
		{
			size_t rr_interval = g_qrs_params.rr_interval;
			float heart_rate = (60.0f * SAMPLE_FREQ) / rr_interval;

			Log_info2("QRS Detected! RR interval: %d ms, Heart Rate: %d BPM",
					(IArg)((rr_interval * 1000) / SAMPLE_FREQ),
					(IArg)heart_rate);
		}
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
