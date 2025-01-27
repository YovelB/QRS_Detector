/******************************************************************************
 * INCLUDES
 *****************************************************************************/

/* Standard C headers */
#include <float.h>  /* For FLT_MAX and FLT_MIN */

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/System.h>

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
/*#include "feature_extract/ecg_detector.h"*/
#include "feature_extract/pqrst_detector.h"

/******************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/* global variable definitions - actual memory allocation happens here */
/* needs to be volatile - accessed by ISR and task concurrently */
volatile float g_ecg_buffer[BUFFER_SIZE]; /* circular buffer for storing ECG samples */
volatile float g_temp_filtered_buffer[BUFFER_SIZE]; /* buffer for fitlered ECG samples */
volatile float g_ecg_filtered_buffer[BUFFER_SIZE*2];
volatile uint16_t g_buffer_index = 0; /* current index pos in buffer */
volatile uint16_t g_temp_index = 0; /* processing index for filtered buffer */
volatile uint16_t g_filtered_index = 0;

/* Pan-Tompkins algorithm buffers and indices */
volatile float g_squared_buffer[BUFFER_SIZE*2]; /* buffer for squared signal output */
uint16_t g_squared_index = 0; /* index for squared buffer */

/* Buffer to store MWI output */
volatile float g_mwi_output[BUFFER_SIZE*2]; /* buffer to store MWI results */
uint16_t g_mwi_output_index = 0; /* index for MWI output buffer */

/* parameters and state tracking for QRS detection algorithm */
qrs_params_t g_qrs_params = {
  .signal_threshold = QRS_MIN_PEAK_AMP,   /* initial detection threshold - set to minimum expected QRS amplitude */
  .noise_threshold = NOISE_THRESHOLD,     /* initial noise floor - filters out low amplitude noise */
  .peak_value = 0.0f,                     /* tracks current peak amplitude - starts at 0 */
  .rr_interval = 0,                       /* time between QRS peaks (in samples) - starts at 0 */
  .last_qrs_index = 0                     /* sample index of last QRS detection - starts at 0 */
};

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
Void ECG_PreprocessingTask(UArg arg0, UArg arg1)
{
  static float signal_min = FLT_MAX;  /* tracks min signal amplitude - starts high */
  static float signal_max = -FLT_MAX; /* tracks max signal amplitude - starts low */

	/* infinite task loop */
	while (1)
	{
		/* wait for signal that new sample is ready */
		Semaphore_pend(g_sample_ready_sem, BIOS_WAIT_FOREVER);

		float input_sample = buffer_read(g_ecg_buffer, g_temp_index, BUFFER_SIZE);

		/* apply derivative filter */
		float filtered_sample = derivative_filter(g_temp_index, input_sample);

    /* apply the Baseline Wander removal filter on the sample */
    /*filtered_sample = baseline_wander_filter(input_sample);*/
    /*filtered_sample = lowfreq_noise_filter(filtered_sample);*/
		
		/* apply the Anti-Aliasing filter on the sample */
		filtered_sample = anti_aliasing_filter(filtered_sample);

    /* apply the QRS enhance filter on the sample */
    /*filtered_sample = qrs_enhance_filter(input_sample);*/

    /* normalize after filtering, before squaring */
    filtered_sample = normalize_signal(filtered_sample, &signal_min, &signal_max) + DC_OFFSET;

		/* write filtered sample to the buffer */
		buffer_write(g_temp_filtered_buffer, &g_temp_index, filtered_sample, BUFFER_SIZE);
		buffer_write(g_ecg_filtered_buffer, &g_filtered_index, filtered_sample, BUFFER_SIZE * 2);

		/* signal that the filtered sample is ready */
		Semaphore_post(g_filtered_ready_sem);
	}
}

Void ECG_FeatureDetectTask(UArg arg0, UArg arg1)
{
	/* execute feature_extract every new sample */
	while (1)
	{

		/* wait for filtered sample from signal conditioning task */
		Semaphore_pend(g_filtered_ready_sem, BIOS_WAIT_FOREVER);

		/* read latest filtered sample */
		float filtered_sample = buffer_read(g_ecg_filtered_buffer, g_squared_index, BUFFER_SIZE*2);

		/* when we have enough samples for one wave cycle */
    if (g_temp_index == BUFFER_SIZE - 1)
		{
			/* perform PQRST detection */
			wave_points_t wave_points = ecg_detect_pqrst(g_temp_filtered_buffer, BUFFER_SIZE);

			/* calculate intervals */
			wave_intervals_t intervals = ecg_calculate_intervals(&wave_points);

			/* validate detection */
			uint8_t quality = ecg_validate_detection(&wave_points, &intervals);

			/* if detection quality is good, log the results */
      if (quality >= 20)
      {
        /* print P Q R S T index and amplitude */
        System_printf("P-wave: idx=%d, amp=%f\n", wave_points.p_idx, wave_points.p_val);
        System_printf("Q-wave: idx=%d, amp=%f\n", wave_points.q_idx, wave_points.q_val);
        System_printf("R-wave: idx=%d, amp=%f\n", wave_points.r_idx, wave_points.r_val);
        System_printf("S-wave: idx=%d, amp=%f\n", wave_points.s_idx, wave_points.s_val);
        System_printf("T-wave: idx=%d, amp=%f\n", wave_points.t_idx, wave_points.t_val);

        /* print intervals */
        System_printf("Intervals: PR=%d ms, QRS=%d ms\n", intervals.pr_interval, intervals.qrs_duration);
        System_printf("Intervals: QT=%d ms, RR=%d ms\n", intervals.qt_interval, intervals.rr_interval);

        /* print quality and heart rate */
        System_printf("Quality=%d, \n", quality);
        System_flush();
      }
		}


		/*/* step 2: squaring operation */
		/*float squared_output = square_signal(filtered_sample);*/
		/*buffer_write(g_squared_buffer, &g_squared_index, squared_output, BUFFER_SIZE*2);*/
		/**/
		/*/* step 3: moving window integration */
		/*float mwi_output = moving_window_integrate(g_mwi_output_index, squared_output);*/
		/*buffer_write(g_mwi_output, &g_mwi_output_index, mwi_output, BUFFER_SIZE*2);*/
		/**/
		/*/* step 4: adaptive peak detection */
		/*if (detect_peaks(&g_qrs_params, mwi_output))*/
		/*{*/
		/*	size_t rr_interval = g_qrs_params.rr_interval;*/
		/*	float heart_rate = (60.0f * SAMPLE_FREQ) / rr_interval;*/
		/**/
		/*	Log_info2("QRS: RR=%d ms, HR=%d BPM", ((rr_interval * 1000) / SAMPLE_FREQ), (IArg)heart_rate);*/
		/**/
		/*	Log_info2("Peak=%d, LastQRS=%d ms", (IArg)(g_qrs_params.peak_value * 1000.0f), g_qrs_params.last_qrs_index);*/
		/*}*/
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
