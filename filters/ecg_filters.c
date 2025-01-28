#include "ecg_filters.h"

#include "config/config.h"
#include "buffers/buffer.h"
#include "Anti_Aliasing_Coeffs.h"

#define DERIVATIVE_POINTS   4       /* number of derivative points */

/* IIR filter state buffers */
static float d_alias[ALIAS_FILTER_STAGES][2] = {{0.0f}};

float derivative_filter(uint16_t curr_index, float sample)
{
  /* signal history buffer */
  static float x[BUFFER_SIZE] = {0};

  /* store new sample */
  x[curr_index] = sample;
  
    /* wait until we have enough samples */
  if (curr_index < DERIVATIVE_POINTS) {
    return 0.0f;
  }

  /* calculate indices with proper wrapping */
  uint16_t i0 = (curr_index + BUFFER_SIZE - 4) % BUFFER_SIZE;   /* n-4 sample */
  uint16_t i1 = (curr_index + BUFFER_SIZE - 3) % BUFFER_SIZE;   /* n-3 sample */
  uint16_t i3 = (curr_index + BUFFER_SIZE - 1) % BUFFER_SIZE;   /* n-1 sample */
  uint16_t i4 = curr_index;                                     /* current sample */

  /* apply 5-point derivative formula: y(n) = (-x(n-4) - 2x(n-3) + 2x(n-1) + x(n))/8 */
  return (-x[i0] - 2.0f * x[i1] + 2.0f * x[i3] + x[i4]) / 8.0f;
}

float normalize_signal(float sample, float* min, float* max) {
  /* update min/max adaptively */
  if (sample > *max) *max = sample;
  if (sample < *min) *min = sample;
  
  /* avoid division by zero */
  float range = *max - *min;
  if (range == 0) return 0;
  
  /* normalize to -1 to 1 range */
  return 2.0f * ((sample - *min) / range) - 1.0f;
}

float iir_biquad_filter(const float (*b)[3], const float (*a)[3], float (*d)[2], uint16_t num_stages, float sample)
{
	float output_y = sample;
	float intermediate = 0.0f;
	uint16_t curr_stage = 1;

	/* process only relevent stages in cascade */
	for (; curr_stage < num_stages - 1; curr_stage +=2)
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

  /* apply gain from previous stage (odd-numbered stages) */
  output_y *= b[curr_stage - 1][0];

	return output_y;
}

float anti_aliasing_filter(float sample)
{
	return iir_biquad_filter(alias_num, alias_den, d_alias, ALIAS_FILTER_STAGES, sample);
}
