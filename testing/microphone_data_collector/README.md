# Microphone Calibration & Testing Pipeline

This directory provides a quick diagnostic routine to test and validate your dual microphone setup.


### 1. Directory structure

```text
testing/microphone_data_collector
├── data/
│   ├── images/                 # Aligned plots: Raw Waves, Filtered Waves, & Cross-Correlation
│   └── raw_files/              # Raw 12-bit stereo data sheets (.csv) captured from the MCU
├── main.c                      # MCU test firmware (sequences through 4 recording passes)
└── read_microphone_data.py     # Python script to ingest, process, and plot the audio tracks
```

### 2. Quick start guide

#### Step 1: Flash the MCU

Replace your standard `main.c` with the one in this directory, compile, and flash your Nucleo board.
Wait for the LED to turn **RED**.

#### Step 2: Run the python script

Open your terminal on your PC, navigate to this directory and launch the ingestion client:

```bash
python3 read_microphone_data.py
```

*(The script automatically scans for Mac/Linux `/dev/tty.usbmodem*` channels).*

#### Step 3: Run the test sequence

Reset your Nucleo board. The firmware will automatically execute **4 test passes** in this order:

1. **Sample 0**: Keep the room completely quiet (sets the noise baseline).
2. **Sample 1**: Make a sound exclusively from the **right**.
3. **Sample 2**: Make a sound directly in the **center**.
4. **Sample 3**: Make a sound exclusively from the **left**.

**LED Cues per pass**: **Flashing blue** (3-second countdown to prepare your position) $\rightarrow$ **Solid green** (20ms active recording window + 1 second text streaming).
When all 4 steps complete, the board locks on a solid **Red LED**.



### 3. Analytical outputs

Once data collection wraps up, check the `./data/` folders:

* `data/raw_files/`: Contains raw values recorded by the microfon that can be used in the DSP testing framework
* `data/images/`: Contains 3-row diagnostic subplots mapping out the raw waveform, the zero-centered speech-band filtered signals (300 Hz - 3000 Hz), and the cross-correlation curve computed in python.

#### What to check in the Terminal:

* **Right Test**: Should give a **positive lag** value.
* **Center Test**: Should give **exactly 0** (or +-1) sample lag.
* **Left Test**: Should give a **negative sample lag** value.