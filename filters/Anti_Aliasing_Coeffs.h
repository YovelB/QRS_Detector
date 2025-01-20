#ifndef Anti_Aliasing_Coeffs_H_
#define Anti_Aliasing_Coeffs_H_

/*
 * Butterworth Low-Pass IIR Filter Coefficients
 * --------------------------------------------
 * Filter Specifications:
 * - Type: Chebyshev Type II Low-Pass
 * - Order: 2nd order
 * - Sampling Frequency (fs): 80 Hz
 * - Frequency Stop (fstop): 25 Hz
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
      0.233581543,              0,              0 
  },
  {
                1,    1.270027399,              1 
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
                1,  -0.4706316292,   0.2344496548 
  },
  {
                1,              0,              0 
  }
};

#endif /* Anti_Aliasing_Coeffs_H_ */
