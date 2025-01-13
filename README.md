## Description
A real-time QRS detector for electrocardiogram (ECG/EKG) 
using digital signal processing techniques, implemented on LCDKMAPL138 development kit.
The system performs continuous sampling of ECG signals at 80 KHz and utilizes digital filtering and
detection algorithms to identify QRS complexes in cardiac waveforms.

## Features
- 80 KHz sampling frequency
- Real-time signal processing
- IIR low-pass filtering (1KHz - 12KHz)
- Interrupt-driven architecture 
- Circular buffer implementation
- TI-RTOS based scheduling

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
