#ifndef PQRST_DETECTOR_H
#define PQRST_DETECTOR_H

#include <stdint.h>
#include "config/config.h"

/* ECG morphology points structure */
typedef struct {
  uint16_t p_idx;   /* P wave position index */
  float p_val;      /* P wave amplitude */
  uint16_t q_idx;   /* Q wave position index */
  float q_val;      /* Q wave amplitude */
  uint16_t r_idx;   /* R wave position index */
  float r_val;      /* R wave amplitude */
  uint16_t s_idx;   /* S wave position index */
  float s_val;      /* S wave amplitude */
  uint16_t t_idx;   /* T wave position index */
  float t_val;      /* T wave amplitude */
} wave_points_t;

/* ECG temporal measurements structure */
typedef struct {
  float pr_interval;    /* PR interval duration (ms) */
  float qrs_duration;   /* QRS complex duration (ms) */
  float qt_interval;    /* QT interval duration (ms) */
  float rr_interval;    /* RR interval duration (ms) */
  float pp_interval;    /* PP interval duration (ms) */
} wave_intervals_t;

/*!
 * @brief Detect PQRST Wave Components
 *
 * analyzes ECG signal buffer to detect characteristic waves:
 * - R peak detection using amplitude threshold
 * - Q and S wave detection within QRS window
 * - P wave detection in PR interval window
 * - T wave detection in QT interval window
 *
 * @param buffer - pointer to ECG signal buffer
 * @param size   - size of the signal buffer
 * @return wave_points_t Structure containing wave locations and amplitudes
 */
wave_points_t ecg_detect_pqrst(volatile const float* buffer, uint16_t size);

/*!
 * @brief Calculate ECG Wave Intervals
 *
 * calculates temporal intervals between detected ECG waves:
 * - PR interval: time from P wave start to QRS start
 * - QRS duration: time from QRS onset to QRS end
 * - QT interval: time from QRS onset to T wave end
 * Note: RR and PP intervals require multiple beats to calculate
 *
 * @param points            - pointer to detected wave points structure
 * @return wave_intervals_t - structure containing calculated intervals in milliseconds
 */
wave_intervals_t ecg_calculate_intervals(const wave_points_t *points);

/*!
 * @brief Validate ECG Wave Detection
 *
 * Performs quality checks on detected waves
 *
 * @param points    - detected PQRST points
 * @param intervals - calculated intervals
 * @return uint8_t  - quality score (0-100) or error code
 */
uint8_t ecg_validatedetection(const wave_points_t *points, const wave_intervals_t *intervals);

#endif /* PQRST_DETECTOR_H */
