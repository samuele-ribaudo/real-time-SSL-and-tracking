#ifndef CONFIG_H
#define CONFIG_H

// Physical quantities
#define MIC_DISTANCE            0.15f     // Spacing between microphones [m]
#define SPEED_OF_SOUND          343.0f    // Speed of sound in air [m/s]
#define SAMPLE_FREQUENCY_HZ     44100     // ADC sampling rate [Hz]

// System settings
#define INACTIVITY_TIMEOUT      5000      // Time to reset to center [ms]
#define AUDIO_BUFFER_SIZE       1024      // Number of samples per channel

// Derived constants
#define MAX_SAMPLE_LAG          ((uint16_t)((MIC_DISTANCE / SPEED_OF_SOUND) * SAMPLE_FREQUENCY_HZ) + 1)
#define TOTAL_DMA_BUFFER_SIZE   (AUDIO_BUFFER_SIZE * 2) // Total size for stereo data

#endif // CONFIG_H