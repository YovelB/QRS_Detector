#ifndef ECG_FILTERS_H
#define ECG_FILTERS_H

#include <stdint.h>

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
 * @param curr_index  - current buffer index, neede to reset intermediate state buffers
 * @param sample      - current input sample to filter
 * @return filtered output with applied gain
 */
float iir_biquad_filter(const float (*a)[3], const float (*b)[3], float (*d)[2],
                        uint16_t num_stages, uint16_t curr_index, float sample);

/*!
 * @brief Baseline wander removal high-pass filter
 *
 * removes low-frequency baseline drift below 0.5Hz caused by:
 * - patient breathing (0.15-0.3Hz)
 * - body movements
 * - poor electrode contact
 * - electrode impedance changes
 *
 * - Stopband frequency (Fstop): 0.9 Hz
 * - Stopband attenuation (Astop): 4 db
 *
 * The filter needs to remove most of the baseline noise at 0.63 Hz
 * and to make sure not to effect P and T waves.
 * There is a percise balance to keep both P and T positive and with great amplitude.
 *
 * @param curr_index  - current buffer index
 * @param sample      - the current input sample to filter
 * @return filtered output sample
 */
float baseline_wander_filter(uint16_t curr_index, float sample);

#endif /* ECG_FILTERS_H */
