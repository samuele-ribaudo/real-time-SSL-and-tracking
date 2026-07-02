# Resources Directory

This directory contains external documentation, reference manuals, and utility scripts used to support the design, hardware setup, and algorithmic implementation of the real-time sound source localization and tracking system.

---

### File registry

| File Name | Type | Description |
| --- | --- | --- |
| **`biquad_coefficients.m`** | MATLAB Script | Script used to design the 6th-order Butterworth bandpass filter (300 Hz - 3000 Hz) and generate the cascaded second-order sections (SOS) biquad coefficients formatted for direct C code integration.|
| **`UM3256_Nucleo64_User_Manual.pdf`** | Hardware Manual | Official STMicroelectronics user manual for the STM32 Nucleo-64 boards (specifically targeting the NUCLEO-U083RC layout), providing pin assignments, jumper configurations, and board schematics.|
| **`UM1940_HAL_LL_Drivers_Manual.pdf`** | Firmware Reference | Comprehensive description of the STM32 HAL and low-layer (LL) drivers, detailing driver files, peripheral handle data structures, and API processing workflows.|