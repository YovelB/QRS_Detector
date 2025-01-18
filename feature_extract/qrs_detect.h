#ifndef QRS_DETECT_H
#define QRS_DETECT_H

#include <stdint.h>
#include "config/config.h"

/*!
 * @brief Derivative filter for QRS detection
 *
 * implements 5-point derivative filter to emphasize QRS complex slopes:
 * y[n] = (1/8) * (-x[n-2] - 2x[n-1] + 2x[n+1] + x[n+2])
 *
 * @param sample - current input sample
 * @param buffer - circular buffer containing samples
 * @param index - current buffer index
 * @return derivative output
 */
float derivative_filter(float *buffer, uint16_t index, float sample);

/*!
 * @brief Square the signal samples
 *
 * performs point-by-point squaring to:
 * - make all data points positive
 * - emphasize larger differences (QRS complexes)
 * - suppress smaller differences (noise)
 *
 * @param sample - input sample to square
 * @return squared output
 */
float square_signal(float sample);

/*!
 * @brief Moving Window Integration
 *
 * performs moving window integration over N samples to obtain signal envelope:
 * - window size ~150ms (30 samples at 200Hz)
 * - provides information about QRS complex duration
 * - helps distinguish T waves from QRS complexes
 *
 * @param sample - current squared sample
 * @param mwi_buffer - buffer for MWI samples
 * @param mwi_index - current MWI buffer index
 * @return integrated output
 */
float moving_window_integrate(float *mwi_buffer, uint16_t *mwi_index,
                                     float sample);

/*!
 * @brief Peak Detection with Adaptive Thresholding
 *
 * detects QRS peaks using adaptive thresholding:
 * - updates signal and noise level estimates
 * - adjusts detection thresholds
 * - handles search windows and refractory periods
 *
 * @param sample -  current integrated sample
 * @param qrs_params - pointer to QRS detection parameters
 * @return 1 if QRS detected, 0 otherwise
 */
int detect_peaks(qrs_params_t *qrs_params, float sample);

#endif /* QRS_DETECT_H */
