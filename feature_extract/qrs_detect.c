#include "qrs_detect.h"

#include "buffers/buffer.h"

/*!
 * @brief QRS Detection Parameters and Thresholds
 *
 * These parameters are optimized for ECG sampling at 80Hz.
 * They are derived from the Pan-Tompkins algorithm with appropriate scaling from the original 200Hz design.
 *
 *
 * @def QRS_MIN_PEAK_AMP
 *      Minimum amplitude threshold for QRS detection (0.3)
 *      - used as initial signal threshold
 *      - assumes normalized signal amplitude (0-1 range)
 *      - peaks below this value are considered noise
 *      - value chosen based on typical QRS to noise ratio in normalized ECG
 *
 * @def NOISE_THRESHOLD
 *      Initial noise level threshold (0.1)
 *      - set to approximately 1/3 of QRS_MIN_PEAK_AMP
 *      - used in adaptive thresholding
 *
 * @def RR_LOW_LIMIT
 *      Minimum allowable RR interval (24 samples)
 *      - represents 300ms at 80Hz sampling rate
 *      - prevents double detection of the same QRS complex
 *      - based on physiological limit (~200 BPM maximum heart rate)
 *      - calculation: 0.300 seconds * 80Hz = 24 samples
 *
 * @def RR_HIGH_LIMIT
 *      Maximum allowable RR interval (166 samples)
 *      - represents 2.075 seconds at 80Hz sampling rate
 *      - used to detect missed beats
 *      - corresponds to minimum heart rate of ~29 BPM
 *      - calculation: 2.075 seconds * 80Hz = 166 samples
 *      - exceeding this limit triggers a forced detection
 *
 * @note: all adaptive thresholds use exponential moving average with
 * alpha = 0.125 (1/8) and beta = 0.875 (7/8) for computational efficiency
 */

#define QRS_MIN_PEAK_AMP    0.3     /* minimum peak amplitude threshold */
#define NOISE_THRESHOLD     0.1     /* initial noise level threshold */
#define RR_LOW_LIMIT        24      /* minimum RR interval (300ms at 80Hz) */
#define RR_HIGH_LIMIT       166     /* missed beat limit (2.075s at 80Hz) */

#define THRESHOLD_ALPHA     0.125f  /* weight for new value (1/8) */
#define THRESHOLD_BETA      0.875f  /* weight for historical value (7/8) */

float derivative_filter(volatile float *buffer, size_t index, float sample)
{
  float output = 0.0;

  /* get delayed samples from buffer */
  float x_n_1 = buffer_read(buffer, (index + BUFFER_SIZE - 1) % BUFFER_SIZE, BUFFER_SIZE);
  float x_n_2 = buffer_read(buffer, (index + BUFFER_SIZE - 2) % BUFFER_SIZE, BUFFER_SIZE);
  float x_n1  = buffer_read(buffer, index + 1, BUFFER_SIZE);
  float x_n2  = buffer_read(buffer, index + 2, BUFFER_SIZE);
  
  /* apply derivative filter */
  output = (-x_n_1 - x_n_1 + x_n1 + x_n2) / 4.0f;
  
  return output;
}

float square_signal(float sample)
{
  return sample * sample;
}

float moving_window_integrate(float* mwi_buffer, size_t* mwi_index, float sample)
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
  static size_t samples_since_last_peak = 0;
  int qrs_detected = 0;

  samples_since_last_peak++;

  /* look for peaks only after refractory period */
  if (samples_since_last_peak > RR_LOW_LIMIT) {
    if (sample > qrs_params->signal_threshold) {
      /* track maximum during potential QRS */
      if (sample > qrs_params->peak_value) {
        qrs_params->peak_value = sample;
      }
      /* detect QRS when finding local maximum or exceeding RR limit */
      else if (sample < qrs_params->peak_value) {
        qrs_detected = 1;
        
        /* update signal threshold */
        qrs_params->signal_threshold = THRESHOLD_ALPHA * qrs_params->peak_value +  THRESHOLD_BETA * qrs_params->signal_threshold;
        
        /* reset measurements */
        samples_since_last_peak = 0;
        qrs_params->peak_value = 0;
      }
    } else if (sample > qrs_params->noise_threshold) {
      /* update noise threshold */
      qrs_params->noise_threshold = THRESHOLD_ALPHA * sample + THRESHOLD_BETA * qrs_params->noise_threshold;
    }
  }

  /* force QRS detection if RR interval too long */
  if (samples_since_last_peak >= RR_HIGH_LIMIT && qrs_params->peak_value > 0) {
    qrs_detected = 1;
    samples_since_last_peak = 0;
    qrs_params->peak_value = 0;
  }

  return qrs_detected;
}
