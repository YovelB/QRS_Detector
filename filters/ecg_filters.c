#include "ecg_filters.h"

#include "config/config.h"
#include "buffers/buffer.h"
#include "Baseline_Wander_Coeffs.h"

float iir_biquad_filter(const float (*b)[3], const float (*a)[3], float (*d)[2],
                        uint16_t num_stages, uint16_t curr_index, float sample)
{
	float output_y = sample;
	float intermediate = 0.0f;
	uint16_t curr_stage = 0;

  /* reset state buffers when index wraps back to 0 */
  if (curr_index == 0) {
    for (; curr_stage < num_stages; curr_stage++) {
      d[curr_stage][0] = 0.0f;
      d[curr_stage][1] = 0.0f;
    }
  }

	/* process all stages in cascade */
	for (curr_stage = 0; curr_stage < num_stages; curr_stage++)
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
	return output_y;
}

float baseline_wander_filter(uint16_t curr_index, float sample)
{
  static float d_baseline[BASELINE_FILTER_STAGES][2] = {{0.0f}};
	return iir_biquad_filter(baseline_num, baseline_den, d_baseline, BASELINE_FILTER_STAGES, curr_index, sample);
}
