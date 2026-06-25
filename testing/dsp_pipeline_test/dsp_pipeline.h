#ifndef DSP_PIPELINE_H
#define DSP_PIPELINE_H

#include <stdint.h>
#include <stdbool.h>

// --- MATLAB OUTPUT OF resources/butterworth.m ---
/*
--- HPF Coefficients (300 Hz) ---
b0 = 0.97369481, b1 = -1.94738962, b2 = 0.97369481
a1 = -1.94669754, a2 = 0.94808171

--- LPF Coefficients (3000 Hz) ---
b0 = 0.02785977, b1 = 0.05571953, b2 = 0.02785977
a1 = -1.47548044, a2 = 0.58691951
*/

typedef struct {
    // filter coefficients comuted in MATLAB
    float b0, b1, b2;
    float a1, a2;

    // history storage variavbles
    float x1, x2;
    float y1, y2;
} biquad_window;


bool check_amplitude_threshold(const uint16_t* left, const uint16_t* right, uint16_t size, const uint16_t threshold);
void remove_dc_offset(const uint16_t *input, int16_t *output, uint16_t size);
float process_window(float input, biquad_window *window);
void bandpass_filter(int16_t *signal, const uint16_t size, biquad_window *hpf, biquad_window *lpf);
int16_t cross_correlate(const int16_t* left, const int16_t* right, uint16_t size, uint16_t max_lag);

#endif // DSP_PIPELINE_H