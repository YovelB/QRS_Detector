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
#include "config/config.h"
#include "buffers/buffer.h"
#include "filters/ecg_filters.h"
#include "feature_extract/qrs_detect.h"

/******************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/* global variable definitions - actual memory allocation happens here */
volatile float g_ecg_buffer[BUFFER_SIZE];             /* circular buffer for storing ECG samples */
volatile float g_ecg_filtered_buffer[BUFFER_SIZE];    /* buffer for fitlered ECG samples */
volatile uint16_t g_buffer_index = 0;                 /* current index pos in buffer */
volatile uint16_t g_process_index = 0;                /* processing index for filtered buffer */

/******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES
 *****************************************************************************/

/******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES
 *****************************************************************************/

/******************************************************************************
 * PRIVATE FUNCTION IMPLEMENTATIONS
 *****************************************************************************/

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

Log_info2("New sample[%u]: %d", (IArg)g_buffer_index, (IArg)((int)(sample * 1000)));

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
    
    float input_sample = buffer_read(g_ecg_buffer, g_process_index, BUFFER_SIZE);
    Log_info2("Processing sample[%d]: %f", (IArg)g_process_index, (IArg)(input_sample * 1000));

    /* apply the Baseline Wander removal filter on the sample */
    float filtered_sample = baseline_wander_filter(input_sample);
    Log_info1("Filtered output: %f", (IArg)(filtered_sample * 1000));

    /* apply the Anti-Aliasing filter on the sample */
    //filtered_sample = anti_aliasing_filter(filtered_sample);
    
    /* apply the QRS enhance filter on the sample */
    filtered_sample = qrs_enhance_filter(filtered_sample);

		/* write filtered sample to the buffer */
		buffer_write(g_ecg_filtered_buffer, &g_process_index, filtered_sample, BUFFER_SIZE);

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
  /* wait for signal that new sample is ready */
  Semaphore_pend(g_filtered_ready_sem, BIOS_WAIT_FOREVER);
  
  /* use differentiation method */  
  //derivative_filter(float *buffer, uint16_t index, float sample);
}

/******************************************************************************
 * MAIN FUNCTION
 *****************************************************************************/
int main(void)
{
	BIOS_start();
	return (0);
}
