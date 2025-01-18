#ifndef Baseline_Wander_Coeffs_H_
#define Baseline_Wander_Coeffs_H_

/*
 * Butterworth High-Pass IIR Filter Coefficients
 * --------------------------------------------
 * Filter Specifications:
 * - Type: Butterworth High-Pass
 * - Order: 2nd order
 * - Sampling Frequency (fs): 80 Hz
 * - Cutoff Frequency (fc): 0.5 Hz
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
     0.9726138711,              0,              0     /* stage 1: 1st order */
  },
  {
                1,             -2,              1     /* stage 2: 3rd order */
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
                1,   -1.944477677,   0.9459779263     /* stage 2: 3rd order */ 
  },
  {
                1,              0,              0     /* stage 3: 1st order */ 
  }
};

#endif /* Baseline_Wander_Coeffs_H_ */
