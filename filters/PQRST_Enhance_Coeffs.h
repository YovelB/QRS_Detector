#ifndef PQRST_Enhance_Coeffs_H
#define PQRST_Enhance_Coeffs_H

/*
 * Butterworth Low-Pass IIR Filter Coefficients
 * --------------------------------------------
 * Filter Specifications:
 * - Type: Butterworth Low-Pass
 * - Order: 2nd order
 * - Sampling Frequency (fs): 80 Hz
 * - Cutoff Frequency (fc): 6 Hz
 * - Filter Structure    : Direct-Form II, Second-Order Sections
 * - Number of Sections  : 2
 * - Stable              : Yes
 * - Linear Phase        : No
 *
 * Generated using MATLAB(R) 24.2 and Signal Processing Toolbox 24.2.
 */

#define ENHANCE_FILTER_STAGES 3

/* numerator coefficients (b) */ 
const int pqrst_enhance_num_order[ENHANCE_FILTER_STAGES] = { 1,3,1 };
const float pqrst_enhance_num[ENHANCE_FILTER_STAGES][3] = {
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
const int pqrst_enhance_den_order[ENHANCE_FILTER_STAGES] = { 1,3,1 };
const float pqrst_enhance_den[ENHANCE_FILTER_STAGES][3] = {
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

#endif /* PQRST_Enhance_Coeffs_H */
