import pandas as pd
import matplotlib.pyplot as plt

# Load Data
raw = pd.read_csv("files/sample_1_raw.csv")
filtered = pd.read_csv("files/sample_1_filtered.csv")
corr_filtered = pd.read_csv("files/correlation_filtered.csv")

# Find the sample lag corresponding to the maximum correlation
max_corr_idx = corr_filtered["Correlation"].idxmax()
sample_lag = corr_filtered["Lag"].iloc[max_corr_idx]

# Create a 3x1 grid
fig, axes = plt.subplots(3, 1, figsize=(10, 12))

# --- Top: Unfiltered Audio ---
axes[0].plot(raw.iloc[:, 0], label="Left Raw")
axes[0].plot(raw.iloc[:, 1], label="Right Raw", alpha=0.7)
axes[0].set_title("Unfiltered Audio Tracks")
axes[0].set_ylabel("Amplitude")
axes[0].legend()

# --- Middle: Filtered Audio ---
axes[1].plot(filtered.iloc[:, 0], label="Left Filtered")
axes[1].plot(filtered.iloc[:, 1], label="Right Filtered", alpha=0.7)
axes[1].set_title("Filtered Audio Tracks (300Hz - 3000Hz)")
axes[1].set_ylabel("Amplitude")
axes[1].legend()

# --- Bottom: Filtered Cross-Correlation ---
axes[2].plot(corr_filtered["Lag"], corr_filtered["Correlation"], color='green')
axes[2].axvline(x=sample_lag, color='red', linestyle='--', label=f'Lag: {sample_lag} samples')
axes[2].set_title("Cross-Correlation (Filtered)")
axes[2].set_xlabel("Lag (samples)")
axes[2].set_ylabel("Correlation Magnitude")
axes[2].grid(True, linestyle='--', alpha=0.6)
axes[2].legend()

plt.tight_layout()
plt.show()