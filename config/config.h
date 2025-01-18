#ifndef ECG_CONFIG_H
#define ECG_CONFIG_H

#include <stdint.h>

/******************************************************************************
 * DEFINES & MACROS
 *****************************************************************************/

/* sampling configuration */
#define SAMPLE_FREQ         80  /* sampling frequency in Hz */
#define BUFFER_SIZE         170 /* number of samples in the input signal */
#define TIMER_PERIOD        1000000 / SAMPLE_FREQ /* timer period in microseconds */

/* QRS Detection Parameters */
#define DERIVATIVE_LENGTH   4 /* length of derivative filter */
#define MWI_WINDOW_SIZE     30 /* moving window integrator size (about 150ms) */
#define QRS_MIN_PEAK_AMP    0.3 /* minimum peak amplitude threshold */
#define RR_LOW_LIMIT        24 /* minimum RR interval (300ms at 80Hz) */
#define RR_HIGH_LIMIT       166 /* missed beat limit (2s at 8Hz) */
#define NOISE_THRESHOLD     0.1 /* initial noise level threshold */

/******************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/
/* global variable declarations with extern */
/* extern - only declares the variable not create them (they are create and allocated in main.c) */
extern volatile float g_ecg_buffer[BUFFER_SIZE];          /* circular buffer for storing ECG samples */
extern volatile float g_ecg_filtered_buffer[BUFFER_SIZE]; /* buffer for filtered ECG samples */
extern volatile uint16_t g_buffer_index;                  /* current index pos in buffer */
extern volatile uint16_t g_process_index;                 /* processing index for filtered buffer */

/******************************************************************************
 * TYPEDEFS
  *****************************************************************************/

/*!
 * @brief QRS detection parameters structure
 */
typedef struct {
  float signal_threshold;       /* adaptive signal peak threshold */
  float noise_threshold;        /* adaptive noise peak threshold */
  float peak_value;             /* current peak value */
  uint16_t rr_interval;         /* current RR interval */
  uint16_t last_qrs_index;        /* buffer index of last QRS detection */
} qrs_params_t;

#endif /* ECG_CONFIG_H */
