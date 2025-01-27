#include "pqrst_detector.h"

/* threshold levels for wave detection - based on percentage of R peak amplitude */
#define R_PEAK_THRESHOLD    0.6f    /* r wave must exceed 60% of max amplitude */
#define P_WAVE_THRESHOLD    0.15f   /* p wave typically 15% of r peak */
#define T_WAVE_THRESHOLD    0.2f    /* t wave typically 20% of r peak */
#define Q_WAVE_THRESHOLD    0.1f    /* q wave minimum -10% of r peak */
#define S_WAVE_THRESHOLD    0.2f    /* s wave minimum -20% of r peak */

/* time windows for detecting wave components at 80hz sampling rate */
#define PR_WINDOW_MAX       16      /* pr intterval max 200ms (16 samples) */
#define QRS_WINDOW_MAX      8       /* qrs complex max 100ms (8 samples) */
#define QT_WINDOW_MAX       32      /* qt interval max 400ms (32 samples) */
#define RR_WINDOW_MIN       48      /* minimum 600ms between r peaks (48 samples) */
#define RR_WINDOW_MAX       120     /* maximum 1500ms between r peaks (120 samples) */

/* helper macros for bounds checking */
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

static uint16_t g_r_idx = 0;
static uint16_t g_q_idx = 0;
static uint16_t g_s_idx = 0;

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
  uint16_t idx = g_r_idx;

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
  uint16_t idx = g_r_idx;

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
  float max_val = -1000.0f;
  uint16_t idx = 0;

  /* search for local maximum before q wave within pr window */
  uint16_t i = MAX(0, g_q_idx - PR_WINDOW_MAX);
  for(; i < g_q_idx; i++) {
    if(buffer[i] > max_val && buffer[i] > P_WAVE_THRESHOLD) {
      max_val = buffer[i];
      idx = i;
    }
  }
  return idx;
}

static uint16_t detect_t_wave(volatile const float* buffer, uint16_t end) {
  float max_val = -1000.0f;
  uint16_t idx = 0;

  /* search for local maximum after s wave within qt window */
  uint16_t i = g_s_idx;
  for(; i < MIN(end, g_s_idx + QT_WINDOW_MAX); i++) {
    if(buffer[i] > max_val && buffer[i] > T_WAVE_THRESHOLD) {
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
  points->r_val = buffer[g_r_idx];

  g_q_idx = detect_q_wave(buffer);
  points->q_idx = g_q_idx;
  points->q_val = buffer[g_q_idx];

  g_s_idx = detect_s_wave(buffer, end);
  points->s_idx = g_s_idx;
  points->s_val = buffer[g_s_idx];

  points->p_idx = detect_p_wave(buffer);
  points->p_val = buffer[points->p_idx];

  points->t_idx = detect_t_wave(buffer, end);
  points->t_val = buffer[points->t_idx];
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

  /* check for physiologically valid qrs duration (80-120 ms) */
  if (intervals->qrs_duration < 80.0f || intervals->qrs_duration > 120.0f)
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
