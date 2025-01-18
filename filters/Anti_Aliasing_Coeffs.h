#ifndef Anti_Aliasing_Coeffs_H_
#define Anti_Aliasing_Coeffs_H_

/*
 * Butterworth Low-Pass IIR Filter Coefficients
 * --------------------------------------------
 * Filter Specifications:
 * - Type: Butterworth Low-Pass
 * - Order: 2nd order
 * - Sampling Frequency (fs): 80 Hz
 * - Cutoff Frequency (fc): 37 Hz
 * - Filter Structure    : Direct-Form II, Second-Order Sections
 * - Number of Sections  : 2
 * - Stable              : Yes
 * - Linear Phase        : No
 *
 * Generated using MATLAB(R) 24.2 and Signal Processing Toolbox 24.2.
 */

#define ALIAS_FILTER_STAGES 5

/* numerator coefficients (b) */ 
const int alias_num_order[ALIAS_FILTER_STAGES] = { 1,3,1,3,1 };
const float alias_num[ALIAS_FILTER_STAGES][3] = {
  {
     0.9053086042,              0,              0       /* stage 1: 1st order */
  },
  {
                1,              2,              1       /* stage 2: 3rd order */
  },
  {
     0.8112239242,              0,              0       /* stage 3: 1st order */
  },
  {
                1,              2,              1       /* stage 4: 3rd order */
  },
  {
                1,              0,              0       /* stage 5: 1st order */
  }
};


/* denominator coefficients (a) */
const int alias_den_order[ALIAS_FILTER_STAGES] = { 1,3,1,3,1 };
const float alias_den[ALIAS_FILTER_STAGES][3] = {
  {
                1,              0,              0       /* stage 1: 1st order */
  },
  {
                1,    1.785253048,    0.835981369       /* stage 2: 3rd order */
  },
  {
                1,              0,              0       /* stage 3: 1st order */
  },
  {
                1,    1.599719644,   0.6451759934       /* stage 4: 3rd order */
  },
  {
                1,              0,              0       /* stage 5: 1st order */
  }
};

#endif /* Anti_Aliasing_Coeffs_H_ */
