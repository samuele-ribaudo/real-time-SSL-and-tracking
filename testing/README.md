# Testing & Data Collection

This directory contains standalone testing frameworks used to validate our Digital Signal Processing (DSP) algorithms and collect real-world microphone data before deploying the full pipeline to the robot.

---

### Directory structure

#### `dsp_pipeline_test/`
A standalone C test environment used to validate the math behind our filtering and cross-correlation functions without needing to flash the board.
* `dsp_pipeline.c` / `dsp_pipeline.h`: The core algorithm files being tested.
* `main.c`: A test harness that feeds pre-recorded arrays into the DSP functions and stores the processed data in `.csv` files.
* `plot_audio.py`: The python script to plot the processed data.


#### `microphone_data_collector/`
Tools used to capture raw ADC data from the microphones and plot it on a PC. This was vital for characterizing room noise and determining the correct filter thresholds.
* **`main.c`**: A simplified STM32 firmware that reads the ADC and streams the raw values over UART/USB to the PC.
* **`read_microphone_data.py`**: A Python script that listens to the serial port, records the incoming data, and saves it.
* **`data/raw_files/`**: CSV files containing the captured ADC readings for various test scenarios:
  * `sample_0_quiet_room_baseline.csv`: Used to calibrate the amplitude threshold.
  * `sample_1_sound_from_the_right.csv`
  * `sample_2_sound_at_center.csv`
  * `sample_3_sound_from_the_left.csv`
* **`data/images/`**: PNG plots generated from the CSV data, allowing us to visually verify the phase shift between the left and right microphones before implementing cross-correlation in C.