#include "qrs_detect.h"

/* depended headers */
#include "buffers/buffer.h"

float derivative_filter(float *buffer, uint16_t index, float sample)
{
  float output = 0.0;

  /* get delayed samples from buffer */
  float x_n_1 = buffer_read(buffer, index - 1, BUFFER_SIZE);
  float x_n_2 = buffer_read(buffer, index - 2, BUFFER_SIZE);
  float x_n1  = buffer_read(buffer, (index + 1) % BUFFER_SIZE, BUFFER_SIZE);
  float x_n2  = buffer_read(buffer, (index + 2) % BUFFER_SIZE, BUFFER_SIZE);
  
  /* apply derivative filter */
  output = (-x_n_2 - 2*x_n_1 + 2*x_n1 + x_n2) / 8.0f;
  
  return output;
}

float square_signal(float sample)
{
  return sample * sample;
}

float moving_window_integrate(float *mwi_buffer, uint16_t *mwi_index, float sample)
{
  static float sum = 0.0f;

  /* substract oldest sample from sum */
  sum -= mwi_buffer[*mwi_index];

  /* add new sample to buffer and sum */
  mwi_buffer[*mwi_index] = sample;
  sum += sample;

  /* update circular buffer index */
  *mwi_index = (*mwi_index + 1) % MWI_WINDOW_SIZE;

  /* return average over window */
  return sum / MWI_WINDOW_SIZE;
}

int detect_peaks(qrs_params_t *qrs_params, float sample)
{
  static uint16_t samples_since_last_peak = 0;
  int qrs_detected = 0;

  samples_since_last_peak++;

  /* look for peaks only after refractory period */
  if (samples_since_last_peak > RR_LOW_LIMIT) 
  {
    if (sample > qrs_params->signal_threshold)
    {
      /* QRS complex candidate found */
      if (sample > qrs_params->signal_threshold)
      {
        if (sample > qrs_params->peak_value)
        {
          qrs_params->peak_value = sample;
        }
        
        /* if we exceeded the high RR interval, this is a QRS */
        if (samples_since_last_peak >= RR_HIGH_LIMIT) {
          qrs_detected = 1;

          /* updated thresholds */
          qrs_params->signal_threshold = 0.125 * qrs_params->peak_value + 0.875 * qrs_params->signal_threshold;

          /* reset measurements */
          samples_since_last_peak = 0;
          qrs_params->peak_value = 0;
        }
      } else {
        /* update noise threshold */
        qrs_params->noise_threshold = 0.125 * sample + 0.875 * qrs_params->noise_threshold;
      }
    }
  }

  return qrs_detected;
}
