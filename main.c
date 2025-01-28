/******************************************************************************
 * INCLUDES
 *****************************************************************************/

/* XDCtools header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/System.h>

/* BIOS header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>

/* user headers */
#include "QRS_Dat_in.h"
#include "config/config.h"
#include "buffers/buffer.h"
#include "filters/ecg_filters.h"
#include "feature_extract/pqrst_detector.h"

/******************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/* needs to be volatile - accessed by ISR and task or two tasks concurrently */
volatile float g_input_buffer[BUFFER_SIZE];               /* circular buffer for storing ECG samples */
volatile uint16_t g_input_index = 0;                   /* current index pos in buffer */

volatile float g_filtered_buffer[BUFFER_SIZE];          /* buffer for fitlered ECG samples */
volatile uint16_t g_filtered_index = 0;                 /* processing index for filtered buffer */

volatile float g_extended_filtered_buffer[EXTENDED_BUFFER_SIZE];
volatile uint16_t g_extended_index = 0;

/******************************************************************************
 * TIMER FUNCTION IMPLEMENTATION
 *****************************************************************************/

/*!
 * @brief Timer64P0 (Timer ID 0) ISR for ECG sampling
 *
 * this ISR is triggred by Timer64P0 (32-bit mode) at 80 sampling frequency so generate interrupts every 12.5ms
 * each interrupt triggered stores a the input sample from QRS_Dat_in to g_input_buffer 
 *
 * timer configuration:
 * - Hardware: Timer64P0 (Timer A - ID 0)
 * - Mode: 32-bit unchained (chained - can be combined with timer B - ID 1 to 64 bit timer)
 * - Start Mode: starts automatically
 * - Run Mode: periodic and continuous
 *
 * @note this function runs in interrupt context and should be kept as short as possible to prevent timing issues.
 */
Void ECG_Timer_ISR(Void)
{
	/* read the new sample from the QRS_IN data */
	float sample = buffer_read(QRS_IN, g_input_index, QRS_BUFFER_SIZE);

	/* write into the cyclic buffer */
	buffer_write(g_input_buffer, &g_input_index, sample, BUFFER_SIZE);

	/* signal that new sample is ready */
	Semaphore_post(g_sample_ready_sem);
}

/******************************************************************************
 * TASK FUNCTION IMPLEMENTATIONS
 *****************************************************************************/

/*!
 * @brief ECG preprocessing task
 *
 * conditions the raw ecg signal through filtering and normalization.
 * runs continuously to process incoming samples and prepare them for feature detection.
 * 
 * processing steps:
 * 1. uses 5 point derivative filter to remove overall noise (including baseline and high frequency noise) to smooth data
 * 1. high pass filter to ehance P, Q, T waves while attenuating S wave (since the derivative distort the PQRST wave)
 * 3. normalizes signal amplitude to -1 and 1 and adding DC_OFFSET to center the signal
 * 4. stores processed samples in filtered buffers and uses semaphore to enable the next task
 *
 * @param arg0 unused task argument
 * @param arg1 unused task argument
 */
Void ECG_PreprocessingTask(UArg arg0, UArg arg1)
{
  static float signal_min = ECG_SIGNAL_MAX;   /* tracks min signal amplitude - starts high */
  static float signal_max = ECG_SIGNAL_MIN;   /* tracks max signal amplitude - starts low */

	while (1)
	{
		/* wait for signal that new sample is ready */
		Semaphore_pend(g_sample_ready_sem, BIOS_WAIT_FOREVER);

		float input_sample = buffer_read(g_input_buffer, g_filtered_index, BUFFER_SIZE);

		/* apply derivative filter */
		float filtered_sample = derivative_filter(g_filtered_index, input_sample);

		/* apply the Anti-Aliasing filter on the sample */
		filtered_sample = anti_aliasing_filter(filtered_sample);

    /* normalize after filtering */
    filtered_sample = normalize_signal(filtered_sample, &signal_min, &signal_max) + DC_OFFSET;

		/* write filtered sample to the buffer */
		buffer_write(g_filtered_buffer, &g_filtered_index, filtered_sample, BUFFER_SIZE);
		buffer_write(g_extended_filtered_buffer, &g_extended_index, filtered_sample, EXTENDED_BUFFER_SIZE);

    /* signal only when a complete wave (BUFFER_SIZE samples) is processed */
    if ((g_filtered_index + 1) % BUFFER_SIZE == 0) {
      Semaphore_post(g_wave_ready_sem);
    }
	}
}

/*!
 * @brief ecg feature detection task
 *
 * detects key ecg wave components and measures their timing.
 * processes blocks as each block is one wave of filtered ecg data 
 * to find p, q, r, s, and t waves and calculates important cardiac intervals.
 *
 * processing steps:
 * 1. collects each PQRST wave of the filtered samples
 * 2. finds wave peaks and valleys
 * 3. calculates timing between waves
 * 4. checks detection quality
 * 5. logs results if quality is good
 *
 * @param arg0 unused task argument
 * @param arg1 unused task argument
 */
Void ECG_FeatureDetectTask(UArg arg0, UArg arg1)
{
  uint16_t curr_wave = 0;                 /* PQRST wave index */

  wave_points_t wave_points = {0};        /* includes points indices and amplitudes of curr wave */
  wave_intervals_t wave_intervals = {0};  /* includes intervals of curr wave */
  uint8_t quality = 0;                    /* quality of the curr measured wave */

	while (1)
	{
		/* wait for filtered sample from signal conditioning task */
		Semaphore_pend(g_wave_ready_sem, BIOS_WAIT_FOREVER);

    /* when we have enough samples for one wave cycle */
    uint16_t start = curr_wave * BUFFER_SIZE; /* start index of the current wave */
    uint16_t stop = start + BUFFER_SIZE;      /* stop index + 1 (size) of the current wave */

    /* perform PQRST detection */
    ecg_detect_pqrst(g_filtered_buffer, start, stop, &wave_points);

    /* calculate intervals */
    ecg_calculate_intervals(&wave_points, &wave_intervals);

    /* validate detection */
    quality = ecg_validate_detection(&wave_points, &wave_intervals);

    /* if detection quality is good, log the results */
    if (quality >= 80)
    {
      /* print curr wave */
      System_printf("\nWave %d:\n", curr_wave + 1);

      /* print P Q R S T index and amplitude */
      System_printf("P-wave: idx=%d, amp=%d mV\n", wave_points.p_idx, (int) wave_points.p_val);
      System_printf("Q-wave: idx=%d, amp=%d mV\n", wave_points.q_idx, (int) wave_points.q_val);
      System_printf("R-wave: idx=%d, amp=%d mV\n", wave_points.r_idx, (int) wave_points.r_val);
      System_printf("S-wave: idx=%d, amp=%d mV\n", wave_points.s_idx, (int) wave_points.s_val);
      System_printf("T-wave: idx=%d, amp=%d mV\n", wave_points.t_idx, (int) wave_points.t_val);
      System_printf("P-previous-wave: idx=%d, R-previous-wave: idx=%d\n", wave_points.prev_p_idx,  wave_points.prev_r_idx);

      /* print intervals */
      System_printf("Intervals: PR=%d ms, QRS=%d ms, QT=%d ms\n", (int) wave_intervals.pr_interval, 
                    (int) wave_intervals.qrs_duration, (int) wave_intervals.qt_interval);
      System_printf("Intervals: RR=%d ms, PP=%d ms\n", (int) wave_intervals.rr_interval, (int) wave_intervals.pp_interval);

      /* print quality and heart rate */
      System_printf("Quality=%d, Heart rate=%d\n", quality, (int) ecg_calculate_heart_rate(&wave_intervals));
      System_flush();
    }

    /* update curr_wave and reset points and intervals */
    curr_wave++;
    if (curr_wave * BUFFER_SIZE >= EXTENDED_BUFFER_SIZE) {
      curr_wave = 0;
      ecg_init(&wave_points, &wave_intervals);
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
