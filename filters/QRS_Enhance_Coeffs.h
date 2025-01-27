#ifndef QRS_Enhance_Coeffs_H_
#define QRS_Enhance_Coeffs_H_

/*
 * Butterworth Band-Pass IIR Filter Coefficients
 * --------------------------------------------
 * Filter Specifications:
 * - Type: Butterworth Band-Pass
 * - Order: 4nd order
 * - Sampling Frequency (fs): 80 Hz
 * - Cutoff Frequency (fc1 - fc2): 5 Hz - 15 Hz
 * - Filter Structure    : Direct-Form II, Second-Order Sections
 * - Number of Sections  : 2
 * - Stable              : Yes
 * - Linear Phase        : No
 *
 * Generated using MATLAB(R) 24.2 and Signal Processing Toolbox 24.2.
 */

#define QRS_FILTER_STAGES 3

/* numerator coefficients (b) */ 
const int qrs_enhance_num_order[QRS_FILTER_STAGES] = { 1,3,1 };
const float qrs_enhance_num[QRS_FILTER_STAGES][3] = {
  {
     0.1752031446,              0,              0 
  },
  {
                1,              0,             -1 
  },
  {
                1,              0,              0 
  }
};

/* denominator coefficients (a) */
const int qrs_enhance_den_order[QRS_FILTER_STAGES] = { 1,3,1 };
const float qrs_enhance_den[QRS_FILTER_STAGES][3] = {
  {
                1,              0,              0 
  },
  {
                1,   -1.628879905,   0.6495937109 
  },
  {
                1,              0,              0 
  }
};

#endif /* QRS_Enhance_Coeffs_H_ */
