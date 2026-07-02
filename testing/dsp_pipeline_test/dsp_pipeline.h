#ifndef DSP_PIPELINE_H
#define DSP_PIPELINE_H

#include <stdint.h>
#include <stdbool.h>

// --- MATLAB OUTPUT OF resources/butterworth.m ---
/*
--- Cascaded Biquad Coefficients (SOS) ---
.b0 = 0.00357387, .b1 = 0.00714774, .b2 = 0.00357387, .a1 = 1.00000000, .a2 = -1.69527851
.b0 = 1.00000000, .b1 = 0.00000000, .b2 = -1.00000000, .a1 = 1.00000000, .a2 = -1.62501451
.b0 = 1.00000000, .b1 = -2.00000000, .b2 = 1.00000000, .a1 = 1.00000000, .a2 = -1.96597588
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
void bandpass_filter(int16_t *signal, const uint16_t size, biquad_window *stages, const uint8_t num_stages);
int16_t cross_correlate(const int16_t* left, const int16_t* right, uint16_t size, uint16_t max_lag);

#endif // DSP_PIPELINE_H