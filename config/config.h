#ifndef ECG_CONFIG_H
#define ECG_CONFIG_H

#include <stdint.h>

/******************************************************************************
 * DEFINES & MACROS
 *****************************************************************************/

#define SAMPLE_FREQ 80                                    /* sampling frequency in Hz */
#define BUFFER_SIZE 85                                    /* number of samples in the input signal */
#define NUM_OF_WAVES 4                                    /* number of repeated PQRST waves */
#define EXTENDED_BUFFER_SIZE BUFFER_SIZE * NUM_OF_WAVES   /* number of samples in the filtered signal */

#define DC_OFFSET 0.15f                                   /* DC offset added to center filtered signal */
#define ECG_SIGNAL_MAX  10.0f                             /* maximum expected ECG signal value in mV */
#define ECG_SIGNAL_MIN -10.0f                             /* minimum expected ECG signal value in mV */

/* helper macros for bounds checking */
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#endif /* ECG_CONFIG_H */
