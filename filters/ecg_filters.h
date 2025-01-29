#ifndef ECG_FILTERS_H
#define ECG_FILTERS_H

#include <stdint.h>

/*!
 * @brief Derivative filter for QRS detection
 *
 * first-order high-pass as 5 point derivative filter derived from central
 * difference approximation: y(n) = (-x(n-4) - 2x(n-3) + 2x(n-1) + x(n))/8
 *
 * provides optimal differentiation at 80Hz sampling for emphasizing rapid
 * voltage changes in QRS complexes while attenuating slower P and T waves
 *
 * @param curr_index  - current buffer index, counter for num of samples
 * @param sample      - current input sample
 * @return derivative output
 */
float derivative_filter(uint16_t curr_index, float sample);

/*!
 * @brief Min-max normalization of ECG signal to range of -1 to 1
 *
 * adaptively tracks signal bounds and normalizes input using:
 * normlaized = 2.0f * ((sample - *min) / (*max - *min)) - 1.0f;
 *
 * @param sample Current input sample
 * @param min - pointer to minimum value
 * @param max - pointer to maximum value
 * @return normalized output
 */
float normalize_signal(float sample, float *min, float *max);

/*!
 * @brief IIR Biquad filter - Direct Form II
 * @param sample - most recent sample
 *
 * Direct Form II Implementation:
 * d[n] = x[n] - a1*d[n-1] - a2*d[n-2]    - intermediate state
 * y[n] = b0*d[n] + b1*d[n-1] + b2*d[n-2] - output
 *
 * where:
 * - d[n] represents intermediate states (delay elements)
 * - b[k] are numerator coefficients (feed-forward)
 * - a[k] are denominator coefficients (feedback)
 * - a0 is normalized to 1.0
 *
 * @param a           - pointer to array of denominator coeffs [a1,a2] for each stage
 * @param b           - pointer to array of numerator coeffs [b0,b1,b2] for each stage
 * @param d           - pointer to array of delay states [d1,d2] for each stage
 * @param num_stages  - number of cascaded biquad filter stages
 * @param sample      - current input sample to filter
 * @return filtered output with applied gain
 */
float iir_biquad_filter(const float (*a)[3], const float (*b)[3], float (*d)[2], uint16_t num_stages, float sample);

/*!
 * @brief PQRST Enhance low-pass filter
 *
 * This IIR filter is designed with a cutoff frequency of 6Hz to:
 * 1. smooth out high-frequency noise introduced by the derivative filter
 * 2. enhance P, Q, and T wave components while reducing S wave amplitude
 *
 * - cutoff frequency: 6 Hz
 *
 * @note this filter was used after a derivative filter to amplify low frequency components
 * like P,T and Q and attenuate high frequency components above 6Hz like the S wave
 * However, it also does introduce and amplify noise at low frequency components
 *
 * @param sample - current input sample to filter
 * @return filtered output
 */
float pqrst_enhance_filter(float sample);

#endif /* ECG_FILTERS_H */
