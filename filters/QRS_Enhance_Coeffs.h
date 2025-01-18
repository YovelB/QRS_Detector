#ifndef QRS_Enhance_Coeffs_H_
#define QRS_Enhance_Coeffs_H_

/*
 * Butterworth Band-Pass IIR Filter Coefficients
 * --------------------------------------------
 * Filter Specifications:
 * - Type: Butterworth Band-Pass
 * - Order: 4nd order
 * - Sampling Frequency (fs): 80 Hz
 * - Cutoff Frequency (fc1 - fc2): 10 Hz - 30 Hz
 * - Filter Structure    : Direct-Form II, Second-Order Sections
 * - Number of Sections  : 2
 * - Stable              : Yes
 * - Linear Phase        : No
 *
 * Generated using MATLAB(R) 24.2 and Signal Processing Toolbox 24.2.
 */

#define QRS_FILTER_STAGES 5

/* numerator coefficients (b) */ 
const int qrs_enhance_num_order[QRS_FILTER_STAGES] = { 1,3,1,3,1 };
const float qrs_enhance_num[QRS_FILTER_STAGES][3] = {
  {
     0.5411961079,              0,              0     /* stage 1: 1st order */
  },
  {
                1,              0,             -1      /* stage 2: 3rd order */
  },
  {
     0.5411961079,              0,              0      /* stage 3: 1st order */
  },
  {
                1,              0,             -1      /* stage 4: 3rd order */
  },
  {
                1,              0,              0      /* stage 5: 1st order */
  }
};

/* denominator coefficients (a) */
const int qrs_enhance_den_order[QRS_FILTER_STAGES] = { 1,3,1,3,1 };
const float qrs_enhance_den[QRS_FILTER_STAGES][3] = {
  {
                1,              0,              0      /* stage 1: 1st order */
  },
  {
                1,   0.9101797342,    0.414213568      /* stage 2: 3rd order */
  },
  {
                1,              0,              0      /* stage 3: 1st order */
  },
  {
                1,  -0.9101797342,    0.414213568      /* stage 4: 3rd order */
  },
  {
                1,              0,              0      /* stage 5: 1st order */
  }
};

#endif /* QRS_Enhance_Coeffs_H_ */
