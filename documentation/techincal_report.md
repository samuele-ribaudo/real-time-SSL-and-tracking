# Technical Report: Real-Time SSL and Tracking System

| Name | Matr. | Email |
|------|---------|-------|
| Samuele Ribaudo | 03821248 | samuele.ribaudo@tum.de |
| Hong Yan Jun  | 03813507 | go75kes@mytum.de |


## 1. Introduction
This project is a robotic "hearing" system that can detect where a sound is coming from and automatically turn to face it.

Using an STM32 microcontroller and two standard microphones, the system acts like a pair of human ears. When a person speaks, the system calculates the sound's origin and uses a servo motor to physically rotate the sensor array toward the speaker. It is designed to mimic a human neck's 180° range of motion. If a sound moves beyond what the system can physically reach, an onboard RGB LED signals a visual warning, acting as a modular trigger that would theoretically tell a larger humanoid robot to rotate its entire body.

---

## 2. MCU Configuration (Samuele)

> The Nucleo board was configured via the STM32CubeMX visual configurator to generate the base C project. Complete peripheral settings are detailed in [section 6 of the repository's REEADME](https://github.com/samuele-ribaudo/real-time-SSL-and-tracking/blob/main/README.md)


### 2.1 Sampling frequency and ADC triggering (Timer 3)

One of the first challenges was determining the optimal microphone sampling frequency, balancing spatial resolution with physical memory constraints.

Given a microphones distance of `d = 0.15 m`, and a speed of sound `v = 343 m/s`, the maximum time delay between channes is `Δt_max = d / v ≈ 437 us`. A standard `16 kHz` audio sampling rate (`t_s = 62.5 us`) yelsd a maximum offset of `437 / 62.5 ≈ 7` samples on each side, for a total range of 14 samples, limiting  our servo angular resolution to a coarse `13°` (180° / 14). To achieve finer angular control, a higher sampling frequency was required.

However, our memory limits us to 1024-sample buffers. Extremely high sampling rates (e.g., `1 MHz`) would fill the buffer in approximately `1 ms`. This short recording window causes two major issues: the lower frequencies of human speech cannot be properly captured for the DSP pipeline (e.g., a `300 Hz` wave requires `≈ 3.3 ms` for a full period), and there is a high risk of only capturing the silences between spoken letters.

To find the sweet spot between accurate fine-grained servo positioning and sufficient audio recording length, we settled on a sampling frequency of `50 kHz`. This yields an audio track of approximately `20 ms`, which is long enough to capture about six full wave periods of human voice at`300 Hz`, while drastically improving the angular resolution to approximately `4°`.
Based on these considerations, Timer 3 was configured to trigger the ADC conversions at a `50 kHz` frequency.

### 2.2 Analog-to-Digital Converter (ADC) & DMA

To guarantee high-speed, non-blocking data acquisition, the 12-bit ADC operates in scan mode, capturing both microphone channels simultaneously on the Timer 3 trigger. Crucially, data is routed directly to memory via Direct Memory Access (DMA). This completely offloads the CPU during the listening phase, ensuring microsecond-level synchronization without computational bottlenecks.

### 2.3 Servo motor actuation (Timer 2)

Timer 2 is configured to generate the 50 Hz PWM signal required for the 180° servo motor. By prescaling the internal 16 MHz clock, we achieve a 1 µs timer resolution. We configured an initial 1.5 ms PWM pulse to guarantee that the servo safely snaps to the exact 90° center position immediately upon power-up, preventing erratic startup movements.

### 2.4 Status indication (GPIO)

Three standard GPIO output pins are mapped to the onboard RGB LED. This provides a simple, interrupt-free method for real-time visual feedback of the system's finite state machine transitions.


---

## 3. Application logic (Samuele)
- **Code Structure:** Use of the `stm32cubeMX_setup(void)` wrapper for a cleaner `main.c` and `config.h` for compile-time parameters.
- **State Machine:** Justification for using an FSM to prevent mechanical noise interference and a brief overview of its implementation.

---

## 4. CAD design (Ryan)
- **CAD Design:** Iterations and 3D printing challenges.

---

## 5. Hardware fabrication (Ryan)
- **Fabrication:** Wiring decisions (soldering common grounds/VCC), embedding 330 Ohm resistors inside the wires for LEDs, broken servo motor, need to use the foam baffles.

---

## 6. Signal filtering & testing framework (Samuele)
- **Testing Framework:** Data collection for microphone validation, quiet room thresholding, and DSP pipeline testing.
- **Signal Filtering:** 
  - Initial attempt: 2nd-order Butterworth (LP + HP) failure *(insert testing framework image here)*.
  - Final approach: 6th-order bandpass filter via MATLAB *(insert voice + 100Hz/6000Hz noise image here)*.

---

## 7. DSP pipeline (Ryan)
- **Acoustic Math:** Threshold check and time-domain cross-correlation algorithm for TDOA.

---

## 8. Conclusions
- Results, problems encountered, and potential future improvements.