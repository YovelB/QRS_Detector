#include "ecg_filters.h"

/* depended headers */
#include "config/config.h"
#include "Baseline_Wander_Coeffs.h"
#include "Anti_Aliasing_Coeffs.h"
#include "QRS_Enhance_Coeffs.h"

float iir_biquad_filter(const float (*b)[3], const float (*a)[3], float (*d)[2], int num_stages, float sample)
{
	/* init variables at the start of the fxn */
	float output_y = sample;
	float intermediate = 0.0f;
	int curr_stage = 0;

	/* process each stage in cascade */
	for (; curr_stage < num_stages; curr_stage++)
	{
		/* get intermidate value */
		/* calculate d[n] = x[n] - a1*d[n-1] - a2*d[n-2] */
		intermediate = output_y - (a[curr_stage][1] * d[curr_stage][0]) - (a[curr_stage][2] * d[curr_stage][1]);

		/* calculate output using b coefficients */
		/* calculate y[n] = b0*d[n] + b1*d[n-1] + b2*d[n-2] */
		output_y = b[curr_stage][0] * intermediate + (b[curr_stage][1] * d[curr_stage][0]) + (b[curr_stage][2] * d[curr_stage][1]);

		/* update state variables */
		d[curr_stage][1] = d[curr_stage][0]; /* d[n-2] = d[n-1] */
		d[curr_stage][0] = intermediate; /* d[n-1] = d[n] */
	}

	/* return filtered output */
	return output_y;
}

float baseline_wander_filter(float sample)
{
	static float d_baseline[BASELINE_FILTER_STAGES][2] = { { 0.0f } };

	return iir_biquad_filter(baseline_num, baseline_den, d_baseline, BASELINE_FILTER_STAGES, sample);
}

float anti_aliasing_filter(float sample)
{
	static float d_alias[ALIAS_FILTER_STAGES][2] = { { 0.0f } };

	return iir_biquad_filter(alias_num, alias_den, d_alias, ALIAS_FILTER_STAGES, sample);
}

float qrs_enhance_filter(float sample)
{
  static float d_qrs[QRS_FILTER_STAGES][2] = { { 0.0f } };

  return iir_biquad_filter(qrs_enhance_num, qrs_enhance_den, d_qrs, QRS_FILTER_STAGES, sample);
}
