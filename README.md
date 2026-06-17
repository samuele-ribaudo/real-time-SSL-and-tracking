# Real-Time Sound Source Localization and Tracking System

| Name | Matr. | Email |
|------|---------|-------|
| Samuele Ribaudo | 03821248 | samuele.ribaudo@tum.de |
| Hong Yan Jun  | 03813507 | go75kes@mytum.de |

## 1. Project overview

This project is a robotic "hearing" system that can detect where a sound is coming from and automatically turn to face it.

Using an STM32 microcontroller and two standard microphones, the system acts like a pair of human ears. When a person speaks, the system calculates the sound's origin and uses a servo motor to physically rotate the sensor array toward the speaker. It is designed to mimic a human neck's 180° range of motion. If a sound moves beyond what the system can physically reach, an onboard RGB LED signals a visual warning, acting as a modular trigger that would theoretically tell a larger humanoid robot to rotate its entire body.

---

## 2. Detailed system description

At a technical level, this project implements an embedded Sound Source Localization (SSL) and tracking system utilizing a NUCLEO-U083RC microcontroller developed within the STM32CubeIDE environment. The system continuously records environmental audio via two analog microphones spaced at a fixed distance. Using cross-correlation algorithms and digital filters, the system calculates the Time Delay of Arrival (TDOA) between the microphones to determine the precise azimuth angle of a human speaker.

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

**`STATE_SETTLE`(Green LED solid):** A non-blocking delay allows the mechanical vibrations of the servo to dissipate before returning to `STATE_LISTEN`.

**`STATE_OUT_OF_BOUNDS` (Red LED flashing):** Triggered when the calculated angle exceeds physical limits. The system flashes the red LED for 1.5 seconds before automatically transitioning back to `STATE_LISTEN`.

**Crucial variables for relative tracking and limits:**

* `current_servo_angle`: Stores the absolute position (0° to 180°) of the platform relative to the room.
* `relative_angle`: The calculated offset from the microphones (-90° to +90°).
* **Limit logic & timeout reset:** The software calculates `new_angle = current_servo_angle + relative_angle`. If `new_angle` exceeds 180° or drops below 0°, the FSM transitions to `STATE_OUT_OF_BOUNDS` to flash the red LED for 1.5 seconds. If no sound is detected within the tracking cone for 5 seconds, the system automatically resets `current_servo_angle = 90` and returns to a center resting position.

---

## 6. Workload distribution & task definition

#### **[Samuele]: Setup, FSM & application logic**
 * **STM32CubeMX peripheral configuration:** Configure the core hardware initialization, including dual ADC channels, circular DMA streams, Hardware Timers (for deterministic sampling triggers and PWM generation), and GPIOs for the RGB LED. Generate the baseline project skeleton.

 * **State machine implementation:** Program the core main.c architecture, managing the sequential transitions between STATE_LISTEN, STATE_COMPUTE, STATE_ACTUATE, STATE_SETTLE, and STATE_OUT_OF_BOUNDS. Handle the DMA interrupt callbacks to orchestrate data handoffs safely.

 * **Geometric angle calculation:** Take the raw sample offset provided by Ryan's algorithm and calculate the relative azimuth angle $\theta$ using the geometric formula: 
 $$\theta = \arcsin\left(\frac{\Delta t \cdot v}{d}\right)$$
   
 * **Actuation, safety controls & limits:** Map the calculated absolute angle to the corresponding Timer PWM duty cycle for the servo. Implement the 0° to 180° software clamping limits, the 1.5-second `STATE_OUT_OF_BOUNDS` non-blocking recovery timer, the 5-second inactivity timeout reset, and the specific RGB color triggers for each state.


#### **[Ryan]: Hardware, fabrication & DSP algorithms**

 * **CAD design & fabrication:** Design the 3D-printable structural mounts for the servo motor, the acoustic foam baffle, and the external "ears."

 * **Physical assembly & circuit wiring:** Build the physical prototype. Wire the two analog microphones, the RGB LED, and the servo motor to the Nucleo board, ensuring solid power delivery and shared grounds.

 * **Manual filtering algorithms:** Write a custom C implementation of a discrete band-pass filter (approx. 300 Hz - 3000 Hz) to isolate human speech frequencies from background noise. Include an amplitude threshold function to keep the pipeline idle when the room is quiet.

 * **Cross-correlation & TDOA:** Implement the time-domain cross-correlation algorithm from scratch. This function will ingest the raw audio buffers provided by Samuele's DMA pipeline, compare them, and output the exact sample offset.


---

## 7. Required materials & sourcing

| **Material component Description** | **Qty** | **Target application / Notes** | **Sourcing / Cost** |
| --- | --- | --- | --- |
| NUCLEO-U083RC Development Board | 1 | Main STM32 microcontroller unit | Lab inventory |
| MAX4466 analog microphones | 2 | Analog microphones for capturing audio | [berrybase ↗](https://www.berrybase.de/elektretmikrofonverstaerker-gy-max4466) (€1,50 cad.)|
| MG996R 180° servo motor | 1 | Positional actuator for rotating the sensor platform | [berrybase ↗](https://www.berrybase.de/waveshare-mg996r-servo-motor-4-8-6v-metallgetriebe-9-11kg-cm-drehmoment-1800-drehwinkel) (€6,50) |
| Acoustic foam block | 1 | High-density foam sponge to block rear audio signals | Probably in the trash |
| RGB LED | 1 | 4 pin LED for FSM state and limit reached indication | Lab inventory |
| PLA filament (3D Printing) | 1 | Material for printing the frame, ears, and servo mounts | Lab inventory |
| Assorted wires & breadboard | 1 | General circuit routing and power distribution | Lab inventory |
