#ifndef ECG_CONFIG_H
#define ECG_CONFIG_H

#include <stdint.h>

/******************************************************************************
 * DEFINES & MACROS
 *****************************************************************************/

#define SAMPLE_FREQ 80              /* sampling frequency in Hz */
#define BUFFER_SIZE 85              /* number of samples in the input signal */
#define NUM_OF_WAVES 4              /* number of repeated PQRST waves */
#define EXTENDED_BUFFER_SIZE BUFFER_SIZE * NUM_OF_WAVES /* number of samples in the filtered signal */

/* threshold levels for wave detection - based on percentage of R peak amplitude */
#define R_PEAK_THRESHOLD    0.6f    /* r wave must exceed 60% of max amplitude */
#define Q_WAVE_THRESHOLD    0.1f    /* q wave minimum -10% of r peak */
#define S_WAVE_THRESHOLD    0.2f    /* s wave minimum -20% of r peak */

/* not used since filtered signal had very small values of P and T waves and barely positive values */
/*#define P_WAVE_THRESHOLD    0.15f    p wave typically 15% of r peak */
/*#define T_WAVE_THRESHOLD    0.2f     t wave typically 20% of r peak */

/* time windows for detecting wave components at 80hz sampling rate */
#define PR_WINDOW_MAX       16      /* pr intterval max 200ms (16 samples) */
#define QRS_WINDOW_MAX      8       /* qrs complex max 100ms (8 samples) */
#define QT_WINDOW_MAX       32      /* qt interval max 400ms (32 samples) */
#define RR_WINDOW_MIN       48      /* minimum 600ms between r peaks (48 samples) */
#define RR_WINDOW_MAX       120     /* maximum 1500ms between r peaks (120 samples) */

/* helper macros for bounds checking */
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#endif /* ECG_CONFIG_H */
