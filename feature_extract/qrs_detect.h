#ifndef QRS_DETECT_H
#define QRS_DETECT_H

#include <stdint.h>
#include "config/config.h"

/*!
 * @brief Square the signal samples
 *
 * performs point-by-point squaring as second stage of Pan-Tompkins algorithm:
 * y[n] = x[n] * x[n]
 *
 * purpose in QRS detection at 80Hz:
 * - makes all data points positive
 * - emphasizes larger differences (QRS complexes ~1mV)
 * - suppresses smaller differences (P, T waves ~0.1-0.3mV)
 * - enhances high-frequency components from derivative stage
 *
 * @param sample - input sample to square
 * @return squared output
 */
float square_signal(float sample);

/*!
 * @brief Moving Window Integration
 *
 * performs moving window integration over N samples to obtain signal envelope:
 * y[n] = (1/N) * Σ(x[n-(N-1)] to x[n])
 * 
 * parameters for 80Hz sampling rate:
 * - window size: 150ms (12 samples)
 * - each sample represents 12.5ms
 * - total integration window spans 150ms
 *
 * purpose in QRS detection:
 * - smooths rectified signal after squaring
 * - provides QRS complex waveform feature information
 * - helps distinguish T waves from QRS complexes
 * - integrates over appropriate period for 80Hz QRS detection
 *
 * @param sample     - current squared sample
 * @return           - integrated output (averaged over window)
 */
float moving_window_integrate(uint16_t curr_index, float sample);

/*!
 * @brief Peak Detection with Adaptive Thresholding for 80Hz ECG
 *
 * Implements QRS peak detection using adaptive thresholding based on Pan-Tompkins:
 * 
 * time constants for 80Hz sampling rate:
 * - refractory period    (RR_LOW_LIMIT)  :   300ms  (24 samples)
 * - maximum RR interval  (RR_HIGH_LIMIT) :   2.075s (166 samples)
 *
 * detection steps:
 * 1. enforces refractory period to prevent double detections
 * 2. tracks potential QRS peaks above signal threshold
 * 3. confirms QRS when peak starts declining
 * 4. updates adaptive thresholds using:
 *    - Signal: threshold = 0.125*peak + 0.875*old_threshold
 *    - Noise:  threshold = 0.125*peak + 0.875*old_threshold
 * 5. forces detection if no QRS found within RR_HIGH_LIMIT
 *
 * @param qrs_params - pointer to QRS detection parameters structure
 * @param sample     - current integrated sample value
 * @return           - 1 if QRS detected, 0 otherwise
 */
uint16_t detect_peaks(qrs_params_t *qrs_params, float sample);

#endif /* QRS_DETECT_H */
