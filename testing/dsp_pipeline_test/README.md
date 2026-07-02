# DSP Pipeline Unit Tests

This directory contains standalone unit tests for the custom digital signal processing pipeline. These tests isolate the hardware-independent algorithmic files (`dsp_pipeline.c` and `dsp_pipeline.h`) from the hardware abstraction layers, enabling rapid simulation and validation on your host computer.

### 1. Directory structure

```text
testing/dsp_pipeline_test/
├── main.c                  # Main test runner
├── dsp_pipeline.h          # Header file defining custom band-pass filters, cross-correlation, and math structures
├── dsp_pipeline.c          # Pure-C manual implementation of DSP algorithms
├── CMakeLists.txt          # Standalone test build configuration for host machine compilation
├── files/                  # Directory containing output and raw data sample files (.csv format)
└── plots/                  # Directory containing some plots we saved from our verification scripts
```

### 2. Prerequisites

Ensure your host environment has the following software installed:
* **Compiler:** `gcc` or `clang` (Natively supported on macOS/Linux)
* **Build System:** `cmake` (version 3.22 or higher) or `make`
* **Python Environments:** `python3` with `matplotlib` and `numpy` installed for plotting scripts.

### 3. Quick start guide

#### Step 1: Build

To build the standalone executable on your host computer, run the following commands from this directory:

```bash
# Create and enter the build directory
mkdir -p build && cd build
# Configure and compile the testing suite
cmake ..
make
```

#### Step 2: Import a raw data file

Save in the directory `files/` a raw data sample file and name it `sample_1_raw.csv`.
For example, you can upload here a file recorded with the microphone data collector framework that are stored inside `testing/microphone_data_collector/data/raw_files`, but remember to rename it.

#### Step 3: Run the c script

Make sure you are in the testing framework directory (`testing/dsp_pipeline_test`) and run:

```bash
./build/dsp_run
```

This executable will ingest your `sample_1_raw.csv`, pass the audio buffers through the discrete band-pass filter (300 Hz - 3000 Hz), compute the cross-correlation function from scratch, and output the results as a new file in the `files/` directory.

#### Step 4: Run the python script

Open your terminal on your PC, navigate to this directory and launch the python script to view the processed data:

```bash
python3 plot_audio.py

```

### 4. How to interpret the data

The output will show 3 images one on top of the other:

1. **The raw signal:** Displays the uncalibrated dual-channel audio data directly imported from your input file, illustrating the ambient background noise and speech ripple before any processing.


2. **The filtered signal:** Shows the waveform after passing through the custom discrete band-pass filter. Frequencies outside human speech parameters (such as low-frequency room hum or high-frequency electrical noise) will be noticeably attenuated.


3. **The cross-correlation:** Plots the cross-correlation function output over the sample lag domain. The highest mathematical peak on this graph explicitly indicates the exact offset between the two microphone elements.