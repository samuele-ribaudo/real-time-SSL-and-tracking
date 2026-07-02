# Code - Real-Time Sound Source Localization

This directory contains the embedded C code for the STM32 NUCLEO-U083RC microcontroller. The software is responsible for sampling audio from two analog microphones via DMA, processing the signals using Digital Signal Processing (DSP) algorithms to find the Time Delay of Arrival (TDOA), and controlling a servo motor and RGB LED based on the localized sound source.

## Code Structure

The codebase is generated and structured using standard STM32Cube/CMake conventions. The following tree structure details the core components implemented for our project:

```text
code/
├── CMakeLists.txt                  # Build configuration file used by the VS Code STM32 extension
├── Core/
│   ├── Inc/
│   │   ├── config.h                 # Global system parameters, thresholds, and calibration constants
│   │   ├── dsp_pipeline.h           # Header declaring custom DSP filtering and cross-correlation functions
│   │   └── main.h                   # Peripheral configuration macros and state machine definitions
│   └── Src/
│       ├── main.c                   # Main application loop, peripheral setup (ADC, DMA, TIM for PWM), and state machine
│       └── dsp_pipeline.c           # Custom DSP pipeline (discrete 300Hz - 3000Hz band-pass filter and cross-correlation)
├── Drivers/
...
```

> Note on Generated Files: all remaining files within the `Core/` directory (such as `stm32u0xx_it.c` for Interrupt Service Routines) and the entire `Drivers/` directory (containing ST's official Hardware Abstraction Layer and ARM CMSIS/DSP libraries) are standard files automatically generated and managed by **STM32CubeMX**.


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
2. In the Run & Debug section, click `Run and Debug`. 
3. Once the debugger starts, click on the arrow to skip it and flash the code onto the board