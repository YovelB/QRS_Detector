#ifndef ECG_CONFIG_H
#define ECG_CONFIG_H

#include <stddef.h>

/******************************************************************************
 * DEFINES & MACROS
 *****************************************************************************/

#define SAMPLE_FREQ 80  /* sampling frequency in Hz */
#define BUFFER_SIZE 170 /* number of samples in the input signal */
#define TIMER_PERIOD 1000000 / SAMPLE_FREQ /* timer period in microseconds */

/*!
 * @def MWI_WINDOW_SIZE
 * moving Window Integration size (12 samples)
 *  
 * at 80Hz sampling rate:
 *  - 12 samples * (1/80Hz) = 150ms window
 *  - matches original Pan-Tompkins window duration of 150ms
 *  - appropriate for typical QRS complex width (80-120ms)
 *  - helps smooth the signal while maintaining distinct QRS features
 *  - prevents merging of QRS complex with adjacent T waves
 */
#define MWI_WINDOW_SIZE     30      /* moving window integrator size (about 150ms) */

/******************************************************************************
 * TYPEDEFS
 *****************************************************************************/

/*!
 * @brief QRS detection parameters structure
 *
 * @struct qrs_params_t
 *    parameters for QRS detection algorithm:
 *    @member signal_threshold  - adaptive threshold for QRS peaks
 *          updates as: 0.125 * peak + 0.875 * previous_threshold
 *
 *    @member noise_threshold   - Adaptive threshold for noise peaks
 *        updates as: 0.125 * peak + 0.875 * previous_threshold
 *
 *    @member peak_value        - current peak amplitude being evaluated
 *
 *    @member rr_interval       - current RR interval in samples
 *
 *    @member last_qrs_index    - buffer index of last detected QRS
 */
typedef struct {
  float signal_threshold;
  float noise_threshold;
  float peak_value;
  size_t rr_interval;
  size_t last_qrs_index;
} qrs_params_t;

/******************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/
/* global variable declarations with extern */
/* extern - only declares the variable not create them (they are create and allocated in main.c) */
extern volatile float g_ecg_buffer[BUFFER_SIZE];              /* circular buffer for storing ECG samples */
extern volatile float g_ecg_filtered_buffer[BUFFER_SIZE];     /* buffer for filtered ECG samples */
extern volatile size_t g_buffer_index;                        /* current index pos in buffer */
extern volatile size_t g_process_index;                       /* processing index for filtered buffer */


#endif /* ECG_CONFIG_H */
