# Real-Time Sound Source Localization and Tracking System

## 1. Project overview

This project is a robotic "hearing" system that can detect where a sound is coming from and automatically turn to face it.

Using an STM32 microcontroller and two standard microphones, the system acts like a pair of human ears. When a person speaks, the system calculates the sound's origin and uses a servo motor to physically rotate the sensor array toward the speaker. It is designed to mimic a human neck's 180° range of motion. If a sound moves beyond what the system can physically reach, an onboard RGB LED signals a visual warning, acting as a modular trigger that would theoretically tell a larger humanoid robot to rotate its entire body.

---

## 2. Detailed system description

At a technical level, this project implements an embedded Sound Source Localization (SSL) and tracking system utilizing a NUCLEO-U083RC microcontroller developed within the VS Code STM32CubeIDE environment. The system continuously records environmental audio via two analog microphones spaced at a fixed distance. Using cross-correlation algorithms and digital filters, the system calculates the Time Delay of Arrival (TDOA) between the microphones to determine the precise azimuth angle of a human speaker.

To overcome the processing limitations of the micro controller and the physical realities of robotics, the system employs clever hardware-software design. It uses an acoustic foam baffle to eliminate the front-back "cone of confusion", calculates all math algorithms from scratch without relying on pre-built DSP libraries, and utilizes a strict "Listen-Compute-Move" state machine to prevent mechanical servo noise from corrupting the audio buffers. A 180° servo motor dynamically actuates the sensor array using a "relative-to-absolute" coordinate mapping to track the sound source.

### Architectural execution modalities

To demonstrate architectural depth without relying on raw register-level manipulation, the project firmware leverages the STM32 Hardware Abstraction Layer (HAL) combined with advanced peripheral configurations:

* **Direct memory access (DMA):** Audio sampling bypasses the CPU entirely. Hardware Timers trigger the ADC to sample both microphones simultaneously directly into memory buffers, ensuring microsecond-level synchronization critical for acoustic math.
* **Manual algorithmic implementation:** All mathematical logic, including the band-pass filtering for human speech, the cross-correlation function for TDOA, and the local-to-global angular geometric conversions, are written completely from scratch in standard C.

---

## 3. Hardware architecture & signal routing

The system relies on fully isolated peripheral connections to the MCU to guarantee non-blocking operations and high-speed data acquisition.

```text
                            +---------------------------+
                            |    STM32U083RC (Nucleo)   |
                            |                           |
+--------------------+      |                           |      +--------------------+
| Analog Mic 1       |----->| [ADC Channel 1]           |      | 180° Servo Motor   |
| (Inside left ear)  |      |        |                  |----->| (Actuation)        |
+--------------------+      |     [DMA]                 | PWM  +--------------------+
             Analog Voltage |        |                  |
                            |        v                  |
+--------------------+      |   [SRAM Buffers]          |      +--------------------+
| Analog Mic 2       |----->|                           |      | RGB LED            |
| (Inside right ear) |      |                           |----->| (Status flag)      |
+--------------------+      | [Timer/PWM Generator]     | GPIO +--------------------+
                            | [GPIO Controller]         |
                            +---------------------------+

```

---

## 4. Components breakdown

**1. Nucleo-U083RC Microcontroller:**
The central processing unit of the system. It handles the high-speed ADC sampling via DMA, executes the mathematical cross-correlation algorithms, maintains the global coordinate tracking variables, and outputs the PWM control signals.

**2. Analog microphones (x2):**
Two independent analog microphones capture the environmental audio. They output a varying analog voltage proportional to the sound waves and are spaced at a known distance d inside the 3D-printed "ears" to allow for acoustic time delay calculations.

**3. Acoustic foam baffle:**
A physical block of sound-absorbing foam mounted directly behind the two microphones. This mechanical solution prevents mathematical front-back ambiguity by physically deafening the system to rear-originating sounds, perfectly matching the 180° physical range of the servo.

**4. 180° PWM servo motor:**
The mechanical actuator that rotates the sensor frame. Using a 180° servo instead of a continuous 360° motor protects the analog microphone wiring from snapping and provides absolute positional control via standard PWM duty cycles.

**5. RGB LED:**
A 4 pin RGB LED that provides real-time visual feedback of the state machine. It acts as a crucial flag for modular robotic design, flashing red when the target escapes the 180° physical field of hearing to trigger a "handoff" to a theoretical humanoid torso.

**6. 3D Printed frame elements:**
Custom PLA components including the structural mounts for the microphones, the acoustic foam, and the servo motor, stylized with two outer "ears".

---

## 5. The FSM & tracking logic breakdown

Because servo gears create immense acoustic noise, the system cannot listen and move at the same time. The finite state machine (FSM) runs sequentially:

**`STATE_LISTEN` (Blue LED pulsing):** The servo is locked in place. The ADC+DMA pipeline is activated to record 1024 samples.
**`STATE_COMPUTE`(Blue LED pulsing):** Audio recording stops. The MCU applies a band-pass filter to check for human speech frequencies. If the audio passes the threshold, cross-correlation is performed to calculate the relative angle.
**`STATE_ACTUATE` (Blue LED solid):** The MCU maps the relative angle to the global coordinate frame and updates the Timer PWM duty cycle.
**`STATE_SETTLE`(Green LED solid):** A non-blocking 100ms delay allows the mechanical vibrations of the servo to dissipate before returning to `STATE_LISTEN`.

**Crucial variables for relative tracking and limits:**

* `current_servo_angle`: Stores the absolute position (0° to 180°) of the platform relative to the room.
* `relative_angle`: The calculated offset from the microphones (-90° to +90°).
* **Limit logic & timeout reset:** The software calculates `new_angle = current_servo_angle + relative_angle`. If `new_angle` exceeds 180° or drops below 0°, the RGB LED flashes **Red** to indicate a range breach. If no sound is detected within the tracking cone for 5 seconds, the system automatically resets `current_servo_angle = 90` and returns to a center resting position.

---

## 6. Workload distribution & task definition

**Group member 1: low level firmware & audio acquisition pipeline**

* **Synchronous ADC configuration:** configure two distinct ADC channels on the STM32 using HAL. Set up a hardware Timer as the trigger source to guarantee a strict, deterministic sampling rate.
* **DMA buffer management:** implement circular DMA requests to pipe the ADC values directly into RAM without CPU intervention. Write the callback functions to flag the FSM when an audio window is ready.
* **Interrupts & callbacks:** write the functions to safely flag the main state machine when a full audio window is ready for processing, ensuring memory isn't overwritten during calculations.

**Group Member 2: digital signal processing & mathematical algorithms**

* **Manual filtering algorithms:** write a custom C implementation of a discrete band-pass filter (approx. 300 Hz - 3000 Hz) to isolate human speech and an amplitude threshold function to keep the system idle in a quiet room.
* **Cross-correlation & TDOA:** implement a time-domain cross-correlation algorithm from scratch to compare the two audio buffers and extract the sample offset.
* **Geometric angle calculation:** write the logic to compute the relative azimuth angle $\theta$ using the formula: $$\theta = \arcsin\left(\frac{\Delta t \cdot v}{d}\right)$$. Integrate the software logic to add this relative offset to the `current_servo_angle`.

**Group member 3: physical hardware, actuation & FSM**

* **CAD design & fabrication:** design the 3D printable structural mounts for the servo motor, the acoustic foam baffle, and the aesthetic ears. Wire the analog microphones and LED to the Nucleo board, ensuring shared grounds and slack in the wiring to allow safe 180° actuation.
* **PWM servo & LED actuation:** configure an STM32 Timer to output a PWM signal to drive the servo. Configure standard GPIO pins to control the Red, Green, and Blue channels of the status LED.
* **State machine:** program the core `main.c` `while(1)` loop containing the Listen, Compute, Actuate, and Settle states. Implement the software clamping limits, the 5 second timeout reset, and the specific RGB color triggers.

---

## 7. Required materials & sourcing

| **Material component Description** | **Qty** | **Target application / Notes** | **Sourcing / Cost** |
| --- | --- | --- | --- |
| NUCLEO-U083RC Development Board | 1 | Main STM32 microcontroller unit | Lab inventory |
| Analog microphones | 2 | Analog microphones for capturing audio |  |
| 180° servo motor | 1 | Positional actuator for rotating the sensor platform | Lab inventory |
| Acoustic foam block | 1 | High-density foam sponge to block rear audio signals |
| RGB LED | 1 | 4 pin LED for FSM state and limit reached indication | Lab inventory |
| PLA filament (3D Printing) | 1 | Material for printing the frame, ears, and servo mounts | Lab inventory |
| Assorted wires & breadboard | 1 | General circuit routing and power distribution | Lab inventory |
