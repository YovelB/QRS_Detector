#ifndef Baseline_Wander_Coeffs_H_
#define Baseline_Wander_Coeffs_H_

/*
 * Chebyshev Type II High-Pass IIR Filter Coefficients
 * --------------------------------------------
 * Filter Specifications:
 * - Type: Chebyshev Type II High-Pass
 * - Order: 2nd order
 * - Sampling Frequency (fs): 80 Hz
 * - Stopband Frequency (Fstop): 0.9 Hz
 * - Stopband Attenuation (Astop): 4 db
 * - Structure: Direct-Form II, Second-Order Sections
 * - Number of Sections  : 1
 * - Stable              : Yes
 * - Linear Phase        : No
 *
 * The filter needs to remove most of the baseline noise at 0.63 Hz
 * and to make sure not to effect P and T waves.
 * There is a percise balance to keep both P and T positive and with great amplitude.
 * 
 * Chebyshev Type II keeps morphology of the signal 
 * while acting like a notch and low pass to remove noise with no ripple.
 * 
 * Generated using MATLAB(R) 24.2 and Signal Processing Toolbox 24.2.
 */

#define BASELINE_FILTER_STAGES 3

/* numerator coefficients (b) */ 
const int baseline_num_order[BASELINE_FILTER_STAGES] = { 1,3,1 };
const float baseline_num[BASELINE_FILTER_STAGES][3] = {
  {
     0.9733407497,              0,              0     /* stage 1: 1st order */
  },
  {
                1,   -1.997501254,              1     /* stage 2: 3rd order */
  },
  {
                1,              0,              0     /* stage 3: 1st order */
  }
};

/* denominator coefficients (a) */
const int baseline_den_order[BASELINE_FILTER_STAGES] = { 1,3,1 };
const float baseline_den[BASELINE_FILTER_STAGES][3] = {
  {
                1,              0,              0     /* stage 1: 1st order */ 
  },
  {
                1,    -1.94353807,   0.9473928213     /* stage 2: 3rd order */
  },
  {
                1,              0,              0     /* stage 3: 1st order */ 
  }
};

#endif /* Baseline_Wander_Coeffs_H_ */
