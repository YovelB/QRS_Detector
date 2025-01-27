#include "qrs_detect.h"

#include "buffers/buffer.h"

/* @note: all adaptive thresholds use exponential moving average with
* alpha = 0.125 (1/8) and beta = 0.875 (7/8) for computational efficiency */
#define THRESHOLD_ALPHA     0.125f  /* weight for new value (1/8) */
#define THRESHOLD_BETA      0.875f  /* weight for historical value (7/8) */

#define MWI_WINDOW_SIZE     12      /* moving window integrator size 12 samples / 80hz = 150ms window */

float square_signal(float sample)
{
  return sample * sample;
}

float moving_window_integrate(uint16_t curr_index, float sample)
{
  static float mwi_buffer[MWI_WINDOW_SIZE] = {0};
  static uint16_t mwi_index = 0;
  static float sum = 0.0f;

  /* subtract oldest sample before overwriting */
  sum -= mwi_buffer[mwi_index];

  /* update circular buffer index */
  buffer_write(mwi_buffer, &mwi_index, sample, MWI_WINDOW_SIZE);
  sum += sample;

  /* return 0 until buffer fills, then return moving average */
  if (curr_index < MWI_WINDOW_SIZE) {
    mwi_index++;
    return 0.0f;
  }

  /* return average over window */
  return sum / MWI_WINDOW_SIZE;
}

uint16_t detect_peaks(qrs_params_t *qrs_params, float sample)
{
  static uint16_t samples_since_last_peak = 0;
  uint16_t qrs_detected = 0;

  samples_since_last_peak++;

  /* look for peaks only after refractory period */
  if (samples_since_last_peak > RR_LOW_LIMIT) {
    if (sample > qrs_params->signal_threshold) {
      /* track maximum during potential QRS */
      if (sample > qrs_params->peak_value)
      {
        qrs_params->peak_value = sample;
      }
      /* detect QRS when finding local maximum */
      else if (sample < qrs_params->peak_value)
      {
        qrs_detected = 1;

        /* calculate RR interval and store last QRS index */
        qrs_params->rr_interval = samples_since_last_peak;
        qrs_params->last_qrs_index += samples_since_last_peak;

        /* update signal threshold */
        qrs_params->signal_threshold = THRESHOLD_ALPHA * qrs_params->peak_value + THRESHOLD_BETA * qrs_params->signal_threshold;

        /* reset measurements */
        samples_since_last_peak = 0;
        qrs_params->peak_value = 0;
      }
    }
  }
  return qrs_detected;
}
