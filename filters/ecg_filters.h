#ifndef ECG_FILTERS_H
#define ECG_FILTERS_H

#include <stddef.h>

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
float iir_biquad_filter(const float (*a)[3], const float (*b)[3], float (*d)[2], size_t num_stages, float sample);

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
 * @param sample The current input sample to filter
 * @return filtered output sample
 */
float baseline_wander_filter(float sample);

/*!
 * @brief Baseline wander removal high-pass filter
 *
 * Removes low-frequency noise below 3Hz cause by noise
 * - Cutoff frequency: 3 Hz
 * 
 * @param sample The current input sample to filter
 * @return filtered output sample
 */
float lowfreq_noise_filter(float sample);

/*!
 * @brief Anti-aliasing low-pass filter
 * 
 * This IIR filter removes high frequency components above Nyquist freq to prevent aliasing effects.
 * With a sampling rate of 80Hz, the Nyquist freq is 40Hz,
 * so any frequencies above this must be attenuated to prevent them from appearing
 * as false lower frequencies in the sampled signal.
 * 
 * - Cutoff frequency: 35 Hz
 * 
 * @param sample The current input sample to filter
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
 * @param sample The current input sample to filter
 * @return filtered output sample
 */
float qrs_enhance_filter(float sample);

#endif /* ECG_FILTERS_H */
