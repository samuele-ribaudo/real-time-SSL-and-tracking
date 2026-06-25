#ifndef CONFIG_H
#define CONFIG_H

// Physical quantities
#define MIC_DISTANCE            0.15f     // Spacing between microphones [m]
#define SPEED_OF_SOUND          343.0f    // Speed of sound in air [m/s]
#define SAMPLE_FREQUENCY_HZ     50000.0f  // ADC sampling rate [Hz]
#define PI                      3.14159   // value of PI

// System settings
#define INACTIVITY_TIMEOUT      5000      // Time to reset to center [ms]
#define AUDIO_BUFFER_SIZE       1024      // Number of samples per channel
#define SETTLING_TIME           100       // Settling time after actuation [ms]

// Hardware settings
#define SERVO_MIN_ANGLE         0.0f      // Minimum servo angle [rad]
#define SERVO_MAX_ANGLE         PI        // Maximum servo angle [rad]
#define SERVO_MIN_PWM           500       // Equivalent to 500 us with  current tim2 setup      
#define SERVO_MAX_PWM           2500      // Equivalent to 2500 us with current tim2 setup
#define QUIET_ROOM_TRESHOLD     500      // Minimum value for a sound to qialify as such

// Derived constants
#define MAX_SAMPLE_LAG          ((uint16_t)((MIC_DISTANCE / SPEED_OF_SOUND) * SAMPLE_FREQUENCY_HZ) + 1)
#define TOTAL_DMA_BUFFER_SIZE   (AUDIO_BUFFER_SIZE * 2) // Total size for stereo data
#define SERVO_CENTER_ANGLE      (SERVO_MAX_ANGLE + SERVO_MIN_ANGLE) / 2.0f

#endif // CONFIG_H