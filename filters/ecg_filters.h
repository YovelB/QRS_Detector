#ifndef ECG_FILTERS_H
#define ECG_FILTERS_H

#include <stdint.h>

/*!
 * @brief Derivative filter for QRS detection
 *
 * first-order high-pass as 5 point derivative filter derived from central
 * difference approximation: y(n) = (-x(n-4) - 2x(n-3) + 2x(n-1) + x(n))/8.
 *
 * provides optimal differentiation at 80Hz sampling for emphasizing rapid
 * voltage changes in QRS complexes while attenuating slower P and T waves.
 *
 * @param curr_index - current buffer index, counter for num of samples
 * @param sample - current input sample
 * @return derivative output
 */
float derivative_filter(uint16_t curr_index, float sample);

/*!
 * @brief Min-max normalization of ECG signal to 0-1 range
 *
 * adaptively tracks signal bounds and normalizes input using:
 * normalized = (sample - min) / (max - min)
 *
 * @param sample Current input sample
 * @param min Pointer to minimum value
 * @param max Pointer to maximum value
 * @return Normalized sample
 */
float normalize_signal(float sample, float *min, float *max);

/*!
 * @brief IIR Biquad filter - Direct Form II
 * @param sample - most recent sample
 *
 * Direct Form II Implementation:
 * d[n] = x[n] - a1*d[n-1] - a2*d[n-2]    // intermediate state
 * y[n] = b0*d[n] + b1*d[n-1] + b2*d[n-2] // output
 *
 * where:
 * - d[n] represents intermediate states (delay elements)
 * - b[k] are numerator coefficients (feed-forward)
 * - a[k] are denominator coefficients (feedback)
 * - a0 is normalized to 1.0
 *
 * @return filtered output sample with applied gain
 */
float iir_biquad_filter(const float (*a)[3], const float (*b)[3], float (*d)[2],
                        uint16_t num_stages, float sample);

/*!
 * @brief Baseline wander removal high-pass filter
 *
 * Removes low-frequency baseline drift below 0.5Hz caused by:
 * - Patient breathing (0.15-0.3Hz)
 * - Body movements
 * - Poor electrode contact
 * - Electrode impedance changes
 *
 * - Cutoff frequency: 0.67 Hz
 *
 * @param sample The current input sample to filter @return filtered output samp
 * e
 * 
 * 
 */
float baseline_wander_filter(float sample);

/*!
 * @brief Anti-aliasing low-pass filter
 *
 * this IIR filter removes high frequency components above Nyquist freq to
 * prevent aliasing effects. with a sampling rate of 80Hz, the Nyquist freq is
 * 40Hz, so any frequencies above this must be attenuated to prevent them from
 * appearing as false lower frequencies in the sampled signal.
 *
 * however, the data is ideal and does not come from adc. hence, doesn't contain any aliasing noise.
 * mostly this filter is used for enhancing Q and R waves and attenuating S and P and T.
 *
 * - cutoff frequency: 6 Hz
 *
 * @param sample - current input sample to filter
 * @return filtered output sample
 */
float anti_aliasing_filter(float sample);

/*!
 * @brief QRS complex enhancement band-pass filter
 *
 * Enhances the QRS complex while attenuating other ECG components.
 * The QRS complex contains frequencies mainly between 10-25Hz.
 * This filter:
 * - Amplifies the QRS complex frequency range
 * - Attenuates P and T waves (below 10Hz)
 * - Reduces high-frequency noise (above 25Hz)
 *
 * - Passband: 10-30 Hz
 *
 * @param sample - current input sample to filter
 * @return filtered output sample
 */
float qrs_enhance_filter(float sample);

#endif /* ECG_FILTERS_H */
