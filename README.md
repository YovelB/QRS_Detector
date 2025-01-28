## Description
Real-time QRS detection system for ECG/EKG signals using Pan-Tompkins algorithm, implemented on LCDKMAPL138. 
The system processes ECG data (85 samples) to detect and analyze PQRST wave components through digital signal processing techniques.

## Features
- Pan-Tompkins QRS detection algorithm
  - bandpass filtering (5-15 Hz)
  - derivative filtering
  - squaring and moving window integration
- PQRST wave analysis
  - peak detection (P, Q, R, S, T waves)
  - amplitude measurement
  - cardiac interval calculation
- Real-time processing
  - 80 Hz sampling frequency
  - circular buffer implementation
  - interrupt-driven architecture
- System Infrastructure
  - TI-RTOS based scheduling
  - IIR low-pass filtering

## Requirements
- LCDKMAPL138 Development Kit
- Code Composer Studio IDE
- OMAPL138 Processor SDK
- TI-RTOS
- XDCtools

## Building
1. Clone this repository
2. Open project in Code Composer Studio
3. Build and flash to LCDKMAPL138 board
