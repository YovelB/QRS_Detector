#ifndef Anti_Aliasing_Coeffs_H_
#define Anti_Aliasing_Coeffs_H_

/*
 * Butterworth Low-Pass IIR Filter Coefficients
 * --------------------------------------------
 * Filter Specifications:
 * - Type: Butterworth Low-Pass
 * - Order: 2nd order
 * - Sampling Frequency (fs): 80 Hz
 * - Cutoff Frequency (fc): 30 Hz
 * - Filter Structure    : Direct-Form II, Second-Order Sections
 * - Number of Sections  : 2
 * - Stable              : Yes
 * - Linear Phase        : No
 *
 * Generated using MATLAB(R) 24.2 and Signal Processing Toolbox 24.2.
 */

#define ALIAS_FILTER_STAGES 3

/* numerator coefficients (b) */ 
const int alias_num_order[ALIAS_FILTER_STAGES] = { 1,3,1 };
const float alias_num[ALIAS_FILTER_STAGES][3] = {
  {
    0.02482790686,              0,              0 
  },
  {
                1,              2,              1 
  },
  {
                1,              0,              0 
  }
};

/* denominator coefficients (a) */
const int alias_den_order[ALIAS_FILTER_STAGES] = { 1,3,1 };
const float alias_den[ALIAS_FILTER_STAGES][3] = {
  {
                1,              0,              0 
  },
  {
                1,   -1.507447362,   0.6067590117 
  },
  {
                1,              0,              0 
  }
};

#endif /* Anti_Aliasing_Coeffs_H_ */
