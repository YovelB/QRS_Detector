/******************************************************************************
 * @file    main.c
 * @brief   Brief description of the file/project
 * @version 1.0
 * @date    YYYY-MM-DD
 * @author  Your Name
 *
 * @copyright Copyright (c) Year Company
 *
 * Additional details about the file/project
 *****************************************************************************/

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
#include "IIR_LowPass_filter_1KHz_12KHz.h"

/******************************************************************************
 * DEFINES & MACROS
 *****************************************************************************/

#define SAMPLE_FREQ     80                      /* sampling frequency in KHz */
#define BUFFER_SIZE     85                      /* number of samples in the input signal */
#define TIMER_PERIOD    1000000 / SAMPLE_FREQ   /* timer period in microseconds */


/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/


/******************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

volatile uint16_t ecg_buffer[BUFFER_SIZE];      /* circular buffer for storing ECG samples */
volatile uint16_t curr_index = 0;               /* current index pos in buffer */

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
 * ISR/SWI FUNCTION IMPLEMENTATIONS
 *************************************w****************************************/

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
 * - Run Mode: periodic and continuous
 *
 * @note this function runs in interrupt context and should be kept as short as possible to prevent timing issues.
 */
static Void ECG_Timer_ISR(Void)
{
  /* sample ADC and store in buffer */
  ecg_buffer[curr_index] = new_sample;
  curr_index++;

  if (curr_index >= BUFFER_SIZE)
  {
    curr_index = 0;
  }
}

/******************************************************************************
 * TASK FUNCTION IMPLEMENTATIONS
 *****************************************************************************/



/******************************************************************************
 * MAIN FUNCTION
 *****************************************************************************/
int main(void)
{
	/* start BIOS */
	BIOS_start();
	return (0);
}
