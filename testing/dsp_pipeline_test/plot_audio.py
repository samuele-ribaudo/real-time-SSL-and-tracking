import pandas as pd
import matplotlib.pyplot as plt

# Load Data
raw = pd.read_csv("files/sample_1_raw.csv")
filtered = pd.read_csv("files/sample_1_filtered.csv")
corr_raw = pd.read_csv("files/correlation_raw.csv")
corr_filtered = pd.read_csv("files/correlation_filtered.csv")

# Create a 2x2 grid
fig, axes = plt.subplots(2, 2, figsize=(14, 8))

# --- Top Left: Unfiltered Audio ---
axes[0, 0].plot(raw.iloc[:, 0], label="Left Raw")
axes[0, 0].plot(raw.iloc[:, 1], label="Right Raw", alpha=0.7)
axes[0, 0].set_title("Unfiltered Audio Tracks")
axes[0, 0].set_ylabel("Amplitude")
axes[0, 0].legend()

# --- Top Right: Filtered Audio ---
axes[0, 1].plot(filtered.iloc[:, 0], label="Left Filtered")
axes[0, 1].plot(filtered.iloc[:, 1], label="Right Filtered", alpha=0.7)
axes[0, 1].set_title("Filtered Audio Tracks (300Hz - 3000Hz)")
axes[0, 1].legend()

# --- Bottom Left: Raw Cross-Correlation ---
axes[1, 0].plot(corr_raw["Lag"], corr_raw["Correlation"], color='purple')
axes[1, 0].set_title("Cross-Correlation (Raw DC Removed)")
axes[1, 0].set_xlabel("Lag (samples)")
axes[1, 0].set_ylabel("Correlation Magnitude")
axes[1, 0].grid(True, linestyle='--', alpha=0.6)

# --- Bottom Right: Filtered Cross-Correlation ---
axes[1, 1].plot(corr_filtered["Lag"], corr_filtered["Correlation"], color='green')
axes[1, 1].set_title("Cross-Correlation (Filtered)")
axes[1, 1].set_xlabel("Lag (samples)")
axes[1, 1].grid(True, linestyle='--', alpha=0.6)

plt.tight_layout()
plt.show()