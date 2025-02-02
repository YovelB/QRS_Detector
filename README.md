## Description
The system processes ECG data to detect and analyze PQRST wave components through digital signal processing techniques, implemented on LCDKMAPL138.

## Features
- signal preprocessing - baseline wander removal:
  - preserves ECG morphology while removing low-frequency drift
  - 2nd order Chebyshev Type II high-pass filter
  - stop-band frequency (Fstop): 0.9 Hz
  - stop-band attenuation (Astop): 4 dB
  - note: doesnt always remove all the baseline noise (depands on the application)

- signal feature extract - PQRST wave detection algorithm
  - peak detection sequence: R → Q,S → P,T
  - amplitude measurement relative to baseline
  - cardiac interval calculation
  - heart rate calculation

- fixed threshold detection:
  - R-wave: 60% of maximum signal amplitude
  - Q-wave: -10% of R wave amplitude
  - S-wave: -20% of R wave amplitude
  - P and T waves: detected in positive amplitude regions
  - fixed time-based validation between detected peaks

- real-time processing
  - 80 Hz sampling frequency
  - circular buffer implementation
  - interrupt-driven architecture

## Requirements
- LCDKMAPL138 Development Kit
- Code Composer Studio IDE
- OMAPL138 Processor SDK
- TI-RTOS
- XDCtools
- UIA (Unified Instrumentation Architecture) tools

## Building
1. Clone this repository
2. Open project in Code Composer Studio
3. Configure target settings:
  - Disable PRU 1 and PRU 2
  - Disable ETB11_0
  - Configure C674X_0 as secondary processor since ARM9_0 is the main processor
4. Build and flash to LCDKMAPL138 board
