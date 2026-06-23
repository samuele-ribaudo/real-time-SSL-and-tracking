import serial
import serial.tools.list_ports
import csv
import os
import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import butter, filtfilt, correlate, correlation_lags

# ==========================================
# CONFIGURATION CONSTANTS
# ==========================================
BAUD_RATE = 115200                 # Matches your BSP configuration
SAMPLING_RATE_HZ = 50000           # 50 kHz sampling rate
EXPECTED_SAMPLES = 1024            # AUDIO_BUFFER_SIZE per channel
TOTAL_FRAMES = 4                   # Quiet, Right, Center, Left
LOW_CUT_HZ = 300.0                 # Target lower voice band cutoff
HIGH_CUT_HZ = 3000.0               # Target upper voice band cutoff

# Human-readable labels for your structured test sequence
SAMPLE_LABELS = {
    0: "Quiet Room (Baseline)",
    1: "Sound from the Right",
    2: "Sound at Center",
    3: "Sound from the Left"
}

def auto_detect_nucleo_port():
    """Scans system ports to discover connected Mac USB modem ports."""
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if "usbmodem" in port.device or "STLink" in port.description or "STM" in port.description:
            print(f"--> Auto-detected Nucleo Board on port: {port.device}")
            return port.device
    return '/dev/tty.usbmodem102'  # Fallback path if auto-detect fails

def butter_bandpass_filter(data, lowcut, highcut, fs, order=4):
    """Applies a standard zero-phase Butterworth bandpass filter using scipy."""
    nyquist = 0.5 * fs
    low = lowcut / nyquist
    high = highcut / nyquist
    b, a = butter(order, [low, high], btype='band')
    return filtfilt(b, a, data) #

def main():
    # 1. Establish Directory Infrastructure
    os.makedirs("data/raw_files", exist_ok=True)
    os.makedirs("data/images", exist_ok=True)
    
    com_port = auto_detect_nucleo_port()
    print(f"Opening serial port {com_port}...")
    try:
        ser = serial.Serial(com_port, BAUD_RATE, timeout=10)
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
        return

    print("\n==============================================================")
    print("SUCCESSFULLY CONNECTED! Reset your Nucleo board to start.")
    print("Follow the LED prompts:")
    print("  1. Flashing Blue -> Prepare speaker/audio position")
    print("  2. Solid Green   -> Capture happening (Speak/Make Noise!)")
    print("==============================================================\n")
    
    captured_frames = []
    current_frame_data = []
    in_frame = False
    
    # 2. Live Serial Ingestion Loop
    try:
        while len(captured_frames) < TOTAL_FRAMES:
            raw_line = ser.readline()
            try:
                line = raw_line.decode('utf-8').strip()
            except UnicodeDecodeError:
                continue

            if not line:
                continue

            if "START" in line: #
                current_frame_data = []
                in_frame = True
                print(f"Receiving transmission for Frame {len(captured_frames)} [{SAMPLE_LABELS[len(captured_frames)]}]...")
                
            elif "END" in line: #
                in_frame = False
                if len(current_frame_data) == EXPECTED_SAMPLES:
                    captured_frames.append(np.array(current_frame_data))
                    print(f"  ✓ Frame {len(captured_frames)-1} successfully received.")
                else:
                    print(f"  ✗ Frame Error: Received {len(current_frame_data)} samples instead of {EXPECTED_SAMPLES}. Retrying frame capture...")
                    
            elif in_frame:
                try:
                    l_val, r_val = map(int, line.split(',')) #
                    current_frame_data.append([l_val, r_val])
                except ValueError:
                    pass
    except KeyboardInterrupt:
        print("\nCollection aborted by user.")
        return
    finally:
        ser.close()
        print("\nSerial link safely closed. Beginning DSP processing pipeline...")

    # 3. DSP Processing Pipeline & Data Saving
    print("\n================== DSP ANALYSIS REPORT ==================")
    
    # Isolate Sample 0 (Quiet Room) to establish the environment's baseline noise floor
    quiet_frame = captured_frames[0]
    quiet_left_centered = quiet_frame[:, 0] - np.mean(quiet_frame[:, 0])
    quiet_left_filtered = butter_bandpass_filter(quiet_left_centered, LOW_CUT_HZ, HIGH_CUT_HZ, SAMPLING_RATE_HZ)
    
    noise_rms = np.sqrt(np.mean(quiet_left_filtered**2)) #
    recommended_threshold = noise_rms * 4.0
    
    print(f"Calculated Background Noise Floor (Filtered RMS): {noise_rms:.2f} ADC Units")
    print(f"Recommended Voice Activity Threshold (4x Noise RMS): {recommended_threshold:.2f} ADC Units")
    print("---------------------------------------------------------")

    time_axis_ms = np.arange(EXPECTED_SAMPLES) / SAMPLING_RATE_HZ * 1000 #

    for idx, frame in enumerate(captured_frames):
        # Extract raw profiles
        raw_left = frame[:, 0]
        raw_right = frame[:, 1]
        
        # Save raw matrices directly out to CSV files
        csv_filename = f"data/raw_files/sample_{idx}_{SAMPLE_LABELS[idx].lower().replace(' ', '_').replace('(', '').replace(')', '')}.csv"
        with open(csv_filename, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(['Left_Channel_Raw', 'Right_Channel_Raw'])
            writer.writerows(frame)
            
        # Zero-center data to remove the DC offset before computing correlation
        left_centered = raw_left - np.mean(raw_left)
        right_centered = raw_right - np.mean(raw_right)
        
        # Apply the Scipy Butterworth Bandpass Filter (300Hz - 3kHz)
        left_filtered = butter_bandpass_filter(left_centered, LOW_CUT_HZ, HIGH_CUT_HZ, SAMPLING_RATE_HZ)
        right_filtered = butter_bandpass_filter(right_centered, LOW_CUT_HZ, HIGH_CUT_HZ, SAMPLING_RATE_HZ)
        
        # Compute Time-Domain Cross-Correlation using Scipy
        cross_corr = correlate(left_filtered, right_filtered, mode='full')
        lags = correlation_lags(len(left_filtered), len(right_filtered), mode='full')
        
        # Identify the peak index where the signals align most closely
        optimal_lag_index = np.argmax(cross_corr)
        calculated_sample_lag = lags[optimal_lag_index]
        
        # Print results to the terminal window
        print(f"Sample {idx} [{SAMPLE_LABELS[idx]}]:")
        print(f"  -> Calculated Sample Lag: {calculated_sample_lag} samples")
        time_delay_us = (calculated_sample_lag / SAMPLING_RATE_HZ) * 1e6
        print(f"  -> Physical Time Delay  : {time_delay_us:.2f} microseconds")
        
        # 4. FIXED: Generate and Save 3 Comparative Subplots (Added Raw Plot Row)
        fig, (ax_raw, ax_wave, ax_corr) = plt.subplots(3, 1, figsize=(11, 9))
        
        # Subplot 1: NEW Raw Signal Waveforms (Oscillating around DC Offset)
        ax_raw.plot(time_axis_ms, raw_left, color='cornflowerblue', alpha=0.8, label='Left Channel (Raw)')
        ax_raw.plot(time_axis_ms, raw_right, color='salmon', alpha=0.8, label='Right Channel (Raw)')
        ax_raw.axhline(y=2048, color='gray', linestyle=':', alpha=0.5, label='1.65V Midpoint (2048)')
        ax_raw.set_title(f"Raw 12-Bit Unfiltered Signals - {SAMPLE_LABELS[idx]}")
        ax_raw.set_ylabel("ADC Value (0-4095)")
        ax_raw.set_ylim(0, 4095) # Explicitly bound to 12-bit ADC range
        ax_raw.grid(True, linestyle=':', alpha=0.6)
        ax_raw.legend(loc='upper right')
        
        # Subplot 2: Filtered Signal Waveform Comparison
        ax_wave.plot(time_axis_ms, left_filtered, color='blue', alpha=0.7, label='Left Channel (Filtered)')
        ax_wave.plot(time_axis_ms, right_filtered, color='orange', alpha=0.7, label='Right Channel (Filtered)')
        ax_wave.set_title("Bandpass Filtered Signals (300Hz - 3kHz, Zero-Centered)")
        ax_wave.set_ylabel("Amplitude (AC Coupled)")
        ax_wave.grid(True, linestyle=':', alpha=0.6)
        ax_wave.legend(loc='upper right')
        
        # Subplot 3: Cross-Correlation Window Curve
        ax_corr.plot(lags, cross_corr, color='purple', label='Cross-Correlation Energy')
        ax_corr.axvline(x=calculated_sample_lag, color='red', linestyle='--', 
                        label=f'Peak Lag Alignment Point ({calculated_sample_lag} samples)')
        ax_corr.set_title("Cross-Correlation Math Model Alignment")
        ax_corr.set_ylabel("Correlation Strength")
        ax_corr.set_xlabel("Sample Lag Offset (Samples)")
        ax_corr.set_xlim(-60, 60) # Tightly track target delay thresholds
        ax_corr.grid(True, linestyle=':', alpha=0.6)
        ax_corr.legend(loc='upper right')
        
        plt.tight_layout()
        image_filename = f"data/images/analysis_sample_{idx}.png"
        plt.savefig(image_filename, dpi=150)
        plt.close()

    print("---------------------------------------------------------")
    print("✓ Data pipeline operations completed successfully.")
    print("  -> Raw data sheets saved under: ./data/raw_files/")
    print("  -> 3-row Diagnostic plots saved under : ./data/images/")
    print("================================================*********\n")

if __name__ == '__main__':
    main()