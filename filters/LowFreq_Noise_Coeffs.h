#ifndef LOWFREQ_NOISE_COEFFS_H_
#define LOWFREQ_NOISE_COEFFS_H_

/*
 * Butterworth High-Pass IIR Filter Coefficients
 * --------------------------------------------
 * Filter Specifications:
 * - Type: Chebshev Type II High-Pass
 * - Order: 2th order
 * - Sampling Frequency (fs): 80 Hz
 * - Frequency Stop (fstop): 1.2 Hz
 * - Maximum Passband Ripple (apass): 10 db
 * - Structure: Direct-Form II, Second-Order Sections
 * - Number of Sections  : 2
 * - Stable              : Yes
 * - Linear Phase        : No
 * 
 * Generated using MATLAB(R) 24.2 and Signal Processing Toolbox 24.2.
 */

#define LOWFREQ_FILTER_STAGES 3

const int lowfreq_num_order[LOWFREQ_FILTER_STAGES] = { 1,3,1 };
const float lowfreq_num[LOWFREQ_FILTER_STAGES][3] = {
  {
     0.9626280069,              0,              0 
  },
  {
                1,   -1.996266842,              1 
  },
  {
                1,              0,              0 
  }
};
const int lowfreq_den_order[LOWFREQ_FILTER_STAGES] = { 1,3,1 };
const float lowfreq_den[LOWFREQ_FILTER_STAGES][3] = {
  {
                1,              0,              0 
  },
  {
                1,   -1.920264125,    0.926654458 
  },
  {
                1,              0,              0 
  }
};

#endif /* LOWFREQ_NOISE_COEFFS_H_ */
