#ifndef Baseline_Wander_Coeffs_H_
#define Baseline_Wander_Coeffs_H_

/*
 * Butterworth High-Pass IIR Filter Coefficients
 * --------------------------------------------
 * Filter Specifications:
 * - Type: Butterworth High-Pass
 * - Order: 2nd order
 * - Sampling Frequency (fs): 80 Hz
 * - Cutoff Frequency (fc): 2 Hz
 * - Structure: Direct-Form II, Second-Order Sections
 * - Number of Sections  : 1
 * - Stable              : Yes
 * - Linear Phase        : No
 * 
 * Generated using MATLAB(R) 24.2 and Signal Processing Toolbox 24.2.
 */

#define BASELINE_FILTER_STAGES 3

/* numerator coefficients (b) */ 
const int baseline_num_order[BASELINE_FILTER_STAGES] = { 1,3,1 };
const float baseline_num[BASELINE_FILTER_STAGES][3] = {
  {
     0.8948585987,              0,              0 
  },
  {
                1,             -2,              1
  },
  {
                1,              0,              0
  }
};

/* denominator coefficients (a) */
const int baseline_den_order[BASELINE_FILTER_STAGES] = { 1,3,1 };
const float baseline_den[BASELINE_FILTER_STAGES][3] = {
  {
                1,              0,              0 
  },
  {
                1,   -1.778631806,   0.8008026481 
  },
  {
                1,              0,              0 
  }
};

#endif /* Baseline_Wander_Coeffs_H_ */
