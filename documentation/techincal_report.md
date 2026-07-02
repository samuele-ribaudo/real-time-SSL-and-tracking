# Technical Report: Real-Time SSL and Tracking System

| Name | Matr. | Email |
|------|---------|-------|
| Samuele Ribaudo | 03821248 | samuele.ribaudo@tum.de |
| Hong Yan Jun  | 03813507 | go75kes@mytum.de |


## 1. Introduction
This project is a robotic "hearing" system that can detect where a sound is coming from and automatically turn to face it.

Using an STM32 microcontroller and two standard microphones, the system acts like a pair of human ears. When a person speaks, the system calculates the sound's origin and uses a servo motor to physically rotate the sensor array toward the speaker. It is designed to mimic a human neck's 180° range of motion. If a sound moves beyond what the system can physically reach, an onboard RGB LED signals a visual warning, acting as a modular trigger that would theoretically tell a larger humanoid robot to rotate its entire body.


## 2. Configuration (Samuele)
- **MCU Configuration:** Rationale behind Timer periods and peripheral setup.


## 3. Application logic (Samuele)
- **Code Structure:** Use of the `stm32cubeMX_setup(void)` wrapper for a cleaner `main.c` and `config.h` for compile-time parameters.
- **State Machine:** Justification for using an FSM to prevent mechanical noise interference and a brief overview of its implementation.

## 4. CAD design (Ryan)
- **CAD Design:** Iterations and 3D printing challenges.

## 5. Hardware fabrication (Ryan)
- **Fabrication:** Wiring decisions (soldering common grounds/VCC), embedding 330 Ohm resistors inside the wires for LEDs, broken servo motor, need to use the foam baffles.

## 6. Signal filtering & testing framework (Samuele)
- **Testing Framework:** Data collection for microphone validation, quiet room thresholding, and DSP pipeline testing.
- **Signal Filtering:** 
  - Initial attempt: 2nd-order Butterworth (LP + HP) failure *(insert testing framework image here)*.
  - Final approach: 6th-order bandpass filter via MATLAB *(insert voice + 100Hz/6000Hz noise image here)*.

## 7. DSP pipeline (Ryan)
- **Acoustic Math:** Threshold check and time-domain cross-correlation algorithm for TDOA.

## 8. Conclusions
- Results, problems encountered, and potential future improvements.