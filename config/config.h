#ifndef ECG_CONFIG_H
#define ECG_CONFIG_H

#include <stdint.h>

/******************************************************************************
 * DEFINES & MACROS
 *****************************************************************************/

#define SAMPLE_FREQ 80            /* sampling frequency in Hz */
#define BUFFER_SIZE 85            /* number of samples in the input signal */
#define EXTENDED_BUFFER_SIZE 170  /* number of samples in the input signal */
#define DC_OFFSET 0.15f           /* DC offset added to center filtered signal */

#define ECG_SIGNAL_MAX  10.0f   /* maximum expected ECG signal value in mV */
#define ECG_SIGNAL_MIN -10.0f   /* minimum expected ECG signal value in mV */

/* helper macros for bounds checking */
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

/******************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/* extern - only declares the variable not create them (they are create and allocated in main.c) */
extern volatile float g_ecg_buffer[BUFFER_SIZE];                /* circular buffer for storing ECG samples */
extern volatile float g_ecg_filtered_buffer[2*BUFFER_SIZE];     /* buffer for filtered ECG samples */
extern volatile uint16_t g_buffer_index;                        /* current index pos in buffer */
extern volatile uint16_t g_process_index;                       /* processing index for filtered buffer */

#endif /* ECG_CONFIG_H */
