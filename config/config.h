#ifndef ECG_CONFIG_H
#define ECG_CONFIG_H

#include <stdint.h>

/******************************************************************************
 * DEFINES & MACROS
 *****************************************************************************/

#define SAMPLE_FREQ 80  /* sampling frequency in Hz */
#define BUFFER_SIZE 85  /* number of samples in the input signal */
#define DC_OFFSET 0.15f /* DC offset added to center filtered signal */

/*!
 * @brief QRS Detection Parameters and Thresholds
 *
 * These parameters are optimized for ECG sampling at 80Hz.
 * They are derived from the Pan-Tompkins algorithm with appropriate scaling from the original 200Hz design.
 *
 * @def QRS_MIN_PEAK_AMP
 *      Minimum amplitude threshold for QRS detection (0.3)
 *      - used as initial signal threshold
 *      - assumes normalized signal amplitude (0-1 range)
 *      - peaks below this value are considered noise
 *      - value chosen based on typical QRS to noise ratio in normalized ECG
 *
 * @def NOISE_THRESHOLD
 *      Initial noise level threshold (0.1)
 *      - set to approximately 1/3 of QRS_MIN_PEAK_AMP
 *      - used in adaptive thresholding
 *
 * @def RR_LOW_LIMIT
 *      Minimum allowable RR interval (24 samples)
 *      - represents 300ms at 80Hz sampling rate
 *      - prevents double detection of the same QRS complex
 *      - based on physiological limit (~200 BPM maximum heart rate)
 *      - calculation: 0.300 seconds * 80Hz = 24 samples
 *
 * @def RR_HIGH_LIMIT
 *      Maximum allowable RR interval (160 samples)
 *      - represents 2 seconds at 80Hz sampling rate
 *      - used to detect missed beats
 *      - corresponds to minimum heart rate of ~29 BPM
 *      - exceeding this limit triggers a forced detection
 *
 */

#define QRS_MIN_PEAK_AMP    0.3     /* minimum peak amplitude threshold */
#define NOISE_THRESHOLD     0.1     /* initial noise level threshold */
#define RR_LOW_LIMIT        24      /* minimum RR interval (300ms at 80Hz) */
#define RR_HIGH_LIMIT       166     /* missed beat limit (2.075s at 80Hz) */

/*!
 * @brief QRS detection parameters structure
 *
 * @struct qrs_params_t
 *    parameters for QRS detection algorithm:
 *    @member signal_threshold  - adaptive threshold for QRS peaks
 *          updates as: 5   + 0.875 * previous_threshold
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
  uint16_t rr_interval;
  uint16_t last_qrs_index;
} qrs_params_t;

/******************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

/* global variable declarations with extern */
/* extern - only declares the variable not create them (they are create and allocated in main.c) */
extern volatile float g_ecg_buffer[BUFFER_SIZE];              /* circular buffer for storing ECG samples */
extern volatile float g_ecg_filtered_buffer[2*BUFFER_SIZE];     /* buffer for filtered ECG samples */
extern volatile uint16_t g_buffer_index;                        /* current index pos in buffer */
extern volatile uint16_t g_process_index;                       /* processing index for filtered buffer */

float g_output[BUFFER_SIZE];
uint16_t g_output_index;

#endif /* ECG_CONFIG_H */
