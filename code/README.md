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
...
```

> Note on generated files: all remaining files and directories (e.g., `stm32u0xx_it.c` in the `Core/` directory for Interrupt Service Routines; the `Drivers/` directory containing ST's official Hardware Abstraction Layer and ARM CMSIS/DSP libraries) are standard files automatically generated and managed by **STM32CubeMX** that we didn't modify.


## Setup, Build, and Flash Instructions

We use **Visual Studio Code** with the official **STM32 VS Code Extension** to build and flash this project.
These instructions have been tested on **Ubuntu 22.04**.

### Prerequisites & dependencies
#### 1. Build tools and ARM toolchain
Install build tools and the ARM toolchain by running the following commands in the terminal:
```bash
sudo apt update && sudo apt upgrade -y
sudo apt install git build-essential cmake ninja-build gcc-arm-none-eabi gdb-multiarch -y
```
#### 2. Flashing tools and USB drivers 
Install flashing tools and USB drivers by running the following command in the terminal:
```bash
sudo apt install openocd stlink-tools -y
sudo usermod -a -G plugdev,dialout $USER
```

> CRITICAL: After running these commands, log out and log back into the Ubuntu session (or restart your PC) for the group changes to take effect.


#### 3. Visual Studio Code & Extension packs
Download and install VS Code from [here](https://code.visualstudio.com/), or by running in the terminal:
```bash
sudo snap install code --classic
```

Open VS Code, go to Extensions tab (`Ctrl+Shift+X`), and search for both `C/C++ Extension Pack` by Microsoft and `STM32CubeIDE` by STMicroelectronic. Install them.


### Cloning the repository
1. Open your terminal.
2. Clone the repository.
```bash
git clone https://github.com/samuele-ribaudo/real-time-SSL-and-tracking.git
```
3. Navigate into the project folder.
```bash
cd real-time-SSL-and-tracking/code/
```
4. Open the project in VS Code.
```bash
code .
```
5. The STM32 Extension should detect the `CMakeLists.txt`. 


### Building
1. Click on the Build gear icon in the bottom left of the screen.
2. Select your build preset (usually `Debug`).
3. Look for a `Build finished with exit code 0` message in the terminal.


### Flashing
1. Connect your NUCLEO-U083RC board to your PC via the USB-C cable.
2. In the Run & Debug section (`Ctrl+Shift+D`), click `Run and Debug`. 
3. Select as debugger `STM32Cube: STM32 Launch STLink GDB Server`.
4. Once the debugger starts, click on the `Continue` arrow (`F5`) and flash the code onto the board.

### Standalone operation
Once the microcontroller has been successfully flashed, the system can operate independently of the PC debugger.
1. Disconnect the NUCLEO board from the PC (removing the USB-C debugging connection).
2. Provide external power to the system by plugging the dedicated wall adapter.
3. The system will initialize automatically. The servo will reset to its default center position, and the microphone array will immediately begin real-time environmental listening and localization.
