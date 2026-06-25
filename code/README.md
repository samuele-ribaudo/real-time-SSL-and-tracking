# Code - Real-Time Sound Source Localization

This directory contains the embedded C code for the STM32 NUCLEO-U083RC microcontroller. The software is responsible for sampling audio from two analog microphones via DMA, processing the signals using Digital Signal Processing (DSP) algorithms to find the Time Delay of Arrival (TDOA), and controlling a servo motor and RGB LED based on the localized sound source.

## Code Structure

The codebase is generated and structured using standard STM32Cube/CMake conventions:

* **`Core/Src/` & `Core/Inc/`**: Contains the main application logic.
  * `main.c` / `main.h`: Initialization of peripherals (ADC, DMA, TIM for PWM, GPIO) and the main super-loop handling the state machine.
  * `dsp_pipeline.c` / `dsp_pipeline.h`: **Our custom DSP implementation.** This contains the discrete band-pass filter (approx. 300 Hz - 3000 Hz) to isolate human speech and the cross-correlation algorithm to calculate the sample offset (TDOA) between the two microphone buffers.
  * `stm32u0xx_it.c`: Interrupt Service Routines (ISRs), specifically for DMA half-transfer and full-transfer callbacks.
* **`Drivers/`**: Hardware Abstraction Layer (HAL) and CMSIS libraries.
  * `STM32U0xx_HAL_Driver/`: ST's official HAL for the U0 series.
  * `CMSIS/DSP/`: ARM Math and DSP libraries (used for optimized vector operations if applicable).
* **`CMakeLists.txt` & `CMakePresets.json`**: Build configuration files used by the VS Code STM32 extension.
* **`STM32U083xx_FLASH.ld` & `STM32U083xx_RAM.ld`**: Linker scripts for memory mapping.

## What We Implemented

1. **Continuous Audio Sampling (ADC + DMA)**: Configured the ADC to read from two analog channels continuously. DMA is used to write these readings into a circular buffer in memory without blocking the CPU.
2. **DSP Pipeline**: 
   * **Amplitude Thresholding**: Checks if the ambient noise exceeds a "quiet room" baseline to wake the system.
   * **Band-pass Filtering**: A custom C implementation of a discrete filter to attenuate noise outside the human vocal range.
   * **Cross-Correlation**: Computes the correlation between the left and right audio buffers in the time domain to find the phase shift (delay).
3. **Actuation (TIM + PWM)**: Based on the calculated delay, we map the TDOA to an azimuth angle and output a PWM signal to the SG90 servo motor to rotate the platform.
4. **Visual Feedback (GPIO)**: If the calculated angle exceeds the 180° physical limit of the servo, an RGB LED is triggered to indicate the target is out of bounds.

---

## Setup, Build, and Flash Instructions

We use **Visual Studio Code** with the official **STM32 VS Code Extension** to build and flash this project.

### Prerequisites & Dependencies
1. **Visual Studio Code**: Download and install [VS Code](https://code.visualstudio.com/).
2. **STM32 VS Code Extension**: Open VS Code, go to the Extensions tab (`Ctrl+Shift+X`), and search for `STM32CubeIDE` by STMicroelectronics. Install it.


### Importing and Building
1. Open VS Code and select `File > Open Folder...` and choose this `code` directory.
2. The STM32 Extension should detect the `CMakeLists.txt`. 
3. In the left activity bar, click on the **STM32 icon** to open the extension panel.
4. Under the **Build** section in the STM32 panel, select your build preset (usually `Debug`).
5. Click **Build**. The integrated CMake and Ninja tools will compile the source code. Look for a `Build finished with exit code 0` message in the terminal.

### Flashing and Running
1. Connect your NUCLEO-U083RC board to your PC via the mini-USB cable.
2. In the STM32 Extension panel, under the **Run & Debug** section, click **Flash Device**. 
3. Open a serial monitor (like PuTTY, TeraTerm, or the integrated VS Code serial monitor) set to `115200` baud rate to view debug prints (if `printf` is retargeted via UART).
4. To debug, click **Debug Device** to flash the code and halt at `main()`. You can now step through the code, inspect variables, and view DMA buffer arrays in real-time.