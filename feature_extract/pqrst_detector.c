#include "pqrst_detector.h"

#include "config/config.h"

/* temp variables to store indices */
static uint16_t g_r_idx = 0;
static uint16_t g_q_idx = 0;
static uint16_t g_s_idx = 0;

void ecg_init(wave_points_t* points, wave_intervals_t* intervals)
{
  if (points) {
    points->p_idx = 0;
    points->p_val = 0.0f;
    points->q_idx = 0;
    points->q_val = 0.0f;
    points->r_idx = 0;
    points->r_val = 0.0f;
    points->s_idx = 0;
    points->s_val = 0.0f;
    points->t_idx = 0;
    points->t_val = 0.0f;
    points->prev_p_idx = 0;
    points->prev_r_idx = 0;
  }

  if (intervals) {
    intervals->pr_interval = 0.0f;
    intervals->qrs_duration = 0.0f;
    intervals->qt_interval = 0.0f;
    intervals->rr_interval = 0.0f;
    intervals->pp_interval = 0.0f;
  }
}
static uint16_t detect_r_peak(volatile const float* buffer, uint16_t start, uint16_t end) {
  float max_val = 0.0f;
  uint16_t idx = 0;

  /* search for maximum value above r peak threshold */
  uint16_t i = start;
  for(; i < end; i++) {
    if(buffer[i] > max_val && buffer[i] > R_PEAK_THRESHOLD) {
      max_val = buffer[i];
      idx = i;
    }
  }
  return idx;
}

static uint16_t detect_q_wave(volatile const float* buffer) {
  float min_val = 0.0f;
  uint16_t idx = 0;

  /* search backwards from r peak within qrs window for local minimum */
  int i = g_r_idx;
  for(; i >= MAX(0, g_r_idx - QRS_WINDOW_MAX); i--) {
    if(buffer[i] < min_val && buffer[i] < -Q_WAVE_THRESHOLD) {
      min_val = buffer[i];
      idx = i;
    }
  }
  return idx;
}

static uint16_t detect_s_wave(volatile const float* buffer, uint16_t end) {
  float min_val = 0.0f;
  uint16_t idx = 0;

  /* search forwards from r peak within qrs window for local minimum */
  uint16_t i = g_r_idx;
  for(; i < MIN(end, g_r_idx + QRS_WINDOW_MAX); i++) {
    if(buffer[i] < min_val && buffer[i] < -S_WAVE_THRESHOLD) {
      min_val = buffer[i];
      idx = i;
    }
  }
  return idx;
}

static uint16_t detect_p_wave(volatile const float* buffer) {
  float max_val = 0.0f;
  uint16_t idx = 0;

  /* search backwards from q peak within pr window for local maximum */
  int i = g_q_idx;
  for(; i >= MAX(0, g_q_idx - PR_WINDOW_MAX); i--) {
    if(buffer[i] > max_val && buffer[i] > 0.0f) {
      max_val = buffer[i];
      idx = i;
    }
  }
  return idx;
}

static uint16_t detect_t_wave(volatile const float* buffer, uint16_t end) {
  float max_val = 0.0f;
  uint16_t idx = 0;

  /* search forwards from s peak within qt window for local maximum */
  uint16_t i = g_s_idx;
  for(; i < MIN(end, g_s_idx + QT_WINDOW_MAX); i++) {
    if(buffer[i] > max_val && buffer[i] > 0.0f) {
      max_val = buffer[i];
      idx = i;
    }
  }
  return idx;
}

void ecg_detect_pqrst(volatile const float* buffer, uint16_t start, uint16_t end, wave_points_t* points) {
  /* store current positions for next calculation first */
  points->prev_r_idx = points->r_idx;
  points->prev_p_idx = points->p_idx;

  /* detect waves in sequence */
  g_r_idx = detect_r_peak(buffer, start, end);
  points->r_idx = g_r_idx;
  points->r_val = buffer[g_r_idx] * 1000.0f; /* V to mV */

  g_q_idx = detect_q_wave(buffer);
  points->q_idx = g_q_idx;
  points->q_val = buffer[g_q_idx] * 1000.0f;

  g_s_idx = detect_s_wave(buffer, end);
  points->s_idx = g_s_idx;
  points->s_val = buffer[g_s_idx] * 1000.0f;

  points->p_idx = detect_p_wave(buffer);
  points->p_val = buffer[points->p_idx] * 1000.0f;

  points->t_idx = detect_t_wave(buffer, end);
  points->t_val = buffer[points->t_idx] * 1000.0f;
}

void ecg_calculate_intervals(const wave_points_t* points, wave_intervals_t* intervals)
{
  float samples_to_ms = 1000.0f / SAMPLE_FREQ;  /* conversion for 80Hz sampling rate */

  /* calculate pr interval (p start to q start) */
  intervals->pr_interval = (points->q_idx - points->p_idx) * samples_to_ms;

  /* calculate qrs duration (q start to s end) */
  intervals->qrs_duration = (points->s_idx - points->q_idx) * samples_to_ms;

  /* calculate qt interval (q start to t end) */
  intervals->qt_interval = (points->t_idx - points->q_idx) * samples_to_ms;

  /* calculate pp and rr intervals if we have previous values */
  if (points->prev_p_idx > 0) {
		intervals->pp_interval = (points->p_idx - points->prev_p_idx) * samples_to_ms;
  }
  if (points->prev_r_idx > 0) {
		intervals->rr_interval = (points->r_idx - points->prev_r_idx) * samples_to_ms;
  }
}

uint8_t ecg_validate_detection(const wave_points_t *points, const wave_intervals_t *intervals)
{
  uint8_t quality_score = 100;

  /* check for missing wave detections */
  if (points->p_idx == 0 || points->q_idx == 0 || points->r_idx == 0 || points->s_idx == 0 || points->t_idx == 0)
  {
    quality_score -= 20;
  }

  /* check for physiologically valid qrs duration (70-110 ms) - assuming adult */
  if (intervals->qrs_duration < 70.0f || intervals->qrs_duration > 110.0f)
  {
    quality_score -= 20;
  }

  /* verify wave sequence order */
  if (!(points->p_idx < points->q_idx && points->q_idx < points->r_idx && points->r_idx < points->s_idx && points->s_idx < points->t_idx))
  {
    quality_score -= 20;
  }

  /* verify r peak is highest amplitude */
  if (points->r_val < points->p_val || points->r_val < points->t_val || points->r_val < -points->q_val || points->r_val < -points->s_val)
  {
    quality_score -= 20;
  }

  /* verify basic wave polarities (q,s negative; p,t positive) */
  if (points->q_val > 0 || points->s_val > 0 || points->p_val < 0 || points->t_val < 0)
  {
    quality_score -= 20;
  }

  return quality_score;
}

float ecg_calculate_heart_rate(const wave_intervals_t *intervals)
{
  float heart_rate = 0.0f;
  
  /* calculate heart rate from RR interval if available */
  if (intervals->rr_interval > 0.0f) {
    /* convert RR interval from ms to beats per minute */
    heart_rate = 60000.0f / intervals->rr_interval;
  }

  /* validate heart rate is within physiological limits (30-200 BPM) */
  if (heart_rate < 30.0f || heart_rate > 200.0f) {
    heart_rate = 0.0f;  /* return 0 for invalid heart rates */
  }
  
  return heart_rate;
}
