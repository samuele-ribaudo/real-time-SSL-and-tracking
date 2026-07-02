#include "dsp_pipeline.h"
#include <stdint.h>
#define FILTER_STAGES 3


// --- MATLAB OUTPUT OF resources/butterworth.m ---
/*
--- Cascaded Biquad Coefficients (SOS) ---
    .b0 = 0.00357387, .b1 = 0.00714774, .b2 = 0.00357387, .a1 = -1.69527851, .a2 = 0.70751554
    .b0 = 1.00000000, .b1 = 0.00000000, .b2 = -1.00000000, .a1 = -1.62501451, .a2 = 0.73868441
    .b0 = 1.00000000, .b1 = -2.00000000, .b2 = 1.00000000, .a1 = -1.96597588, .a2 = 0.96748119
*/


typedef struct {
    // filter coefficients comuted in MATLAB
    float b0, b1, b2;
    float a1, a2;

    // history storage variavbles
    float x1, x2;
    float y1, y2;
} biquad_window;


/**
 * @brief Scans the raw audio buffers to check for a valid sound event.
 * @param[in] left   Pointer to Left channel ADC buffer.
 * @param[in] right  Pointer to Right channel ADC buffer.
 * @param[in] size   Total number of samples in the buffers.
 * @return true if peak-to-peak amplitude exceeds noise floor, false if room is quiet.
 */
static bool check_amplitude_threshold(const uint16_t* left, const uint16_t* right, uint16_t size, const uint16_t threshold) {

    uint16_t left_min = 4095; // Max 12-bit ADC value
    uint16_t left_max = 0;
    uint16_t right_min = 4095;
    uint16_t right_max = 0;

    // Find the min and max for both channels
    for (uint16_t i = 0; i < size; i++) {
        if (left[i] < left_min) left_min = left[i];
        if (left[i] > left_max) left_max = left[i];
        
        if (right[i] < right_min) right_min = right[i];
        if (right[i] > right_max) right_max = right[i];
    }

    // Calculate peak-to-peak amplitudes
    uint16_t left_amplitude = left_max - left_min;
    uint16_t right_amplitude = right_max - right_min;

    // Check if either channel exceeds the threshold
    if (left_amplitude > threshold || right_amplitude > threshold) {
        return true; 
    }

    return false; // Room is quiet
}


/**
 * @brief Calculates the mean of the input array and subtracts it to center the signal at 0.
* @param[in] input   pointer to the array storng the raw data
 * @param[out] output  pointer to the array storng the shifted data
 * @return none
 */
static void remove_dc_offset(const uint16_t *input, int16_t *output, uint16_t size){

    int32_t sum = 0;
    for (uint16_t i = 0; i < size; i++) {
        sum += input[i];
    }
    int16_t mean = sum / size;
    for (uint16_t i = 0; i < size; i++) {
        output[i] = (int16_t)input[i] - mean;
    }
}


/**
 * @brief process one window of rhe biquad filter
 * @param[in] input input value
 * @param[in] window  biquad window storing coefficients and memory values
 * @return the filtered output
 */
static float process_window(float input, biquad_window *window){
    // compute the core difference equation
    float output = (window->b0 * input) +
                    (window->b1 * window->x1) +
                    (window->b2 * window->x2) -
                    (window->a1 * window->y1) -
                    (window->a2 * window->y2);
    
    // update input history
    window->x2 = window->x1;
    window->x1 = input;

    // update output history
    window->y2 = window->y1;
    window->y1 = output;

    return output;
}


/**
 * @brief apply a low pass filter and an high pass filter to obtain a bandpass filter to an array
 * @param[in, out] signal   pointer to the array storng the unfiltered data, that will contain the filtered one
 * @param[in] size      length of the data arrays
 * @param[in] stages    pointer to the array of biquad windows
 * @param[in] num_stages number of biquad stages
 * @return none
 */
static void bandpass_filter(int16_t *signal, const uint16_t size, biquad_window *stages, const uint8_t num_stages){
    for(int i = 0; i < size; i++){
        // convert integer sample to float
        float current_sample = (float) signal[i];

        for(uint8_t stage = 0; stage < num_stages; stage++)
            current_sample = process_window(current_sample, &stages[stage]);

        // Calmping to prevent integer overflow
        if(current_sample > 32767.0f) current_sample = 32767.0f;
        if(current_sample < -32768.0f) current_sample = -32768.0f;

        // write the data in the output array
        signal[i] = (int16_t) current_sample;
    }
}

/**
 * @brief Computes mathematical alignment to find the arrival delay.
 * @param[in] left     Pointer to filtered Left channel buffer.
 * @param[in] right    Pointer to filtered Right channel buffer.
 * @param[in] size     Total number of samples in the buffers.
 * @param[in] max_lag  The physical search window limit.
 * @return The calculated index sample offset (lag value between -max_lag and +max_lag).
 */
static int16_t cross_correlate(const int16_t* left, const int16_t* right, uint16_t size, uint16_t max_lag) {

    int16_t best_lag = 0;
    int64_t max_correlation = -1; // Initialize to a very low number

    // Loop through the possible lags: from -max_lag to +max_lag
    for (int16_t lag = -max_lag; lag <= max_lag; lag++) {
        int64_t current_correlation = 0;
        
        // Loop through the overlapping area of the arrays
        for (uint16_t i = 0; i < size; i++) {
            int16_t j = i + lag;
            
            // Ensure we stay within the array bounds
            if (j >= 0 && j < size) {
                // To prevent overflow from DC offset, it's common practice to mean-center the data.
                // However, since we apply a band-pass filter before this function, 
                // the DC offset should be minimal. We can proceed with the direct sum of products.
                
                // Using int32_t to avoid overflow during multiplication of 16-bit values
                int32_t left_val = (int32_t)left[i];
                int32_t right_val = (int32_t)right[j];
                
                current_correlation += left_val * right_val;
            }
        }
        
        // Update best lag if current correlation is higher
        if (current_correlation > max_correlation) {
            max_correlation = current_correlation;
            best_lag = lag;
        }
    }
    
    return best_lag;
}



// --- PUBLIC INTERFACE FUNCTION ---

bool DSP_pipeline(int16_t* out_sample_offset, const uint16_t quiet_room_treshold,
                  const uint16_t* left_mic, const uint16_t* right_mic, 
                  const uint16_t size, const uint16_t max_sample_lag) 
{
    // 1. Detect: check if a sound event is present in the raw audio buffers
    if(!check_amplitude_threshold(left_mic, right_mic, size, quiet_room_treshold)){
        return false; // Exit early to save CPU power
    }

    // 2. Use int16_t and static to prevent Stack Overflow
    static int16_t left_clean[1024];
    static int16_t right_clean[1024];

    // 3. Remove dc offset
    remove_dc_offset(left_mic, left_clean, size);
    remove_dc_offset(right_mic, right_clean, size);

    // 4. Apply band pass filter;

    // initialize the biquad filter windows
    biquad_window stages[FILTER_STAGES] = {
        {0.00357387f, 0.00714774f, 0.00357387f, -1.69527851f, 0.70751554f, 0, 0, 0, 0},
        {1.00000000f, 0.00000000f, -1.00000000f, -1.62501451f, 0.73868441f, 0, 0, 0, 0},
        {1.00000000f, -2.00000000f, 1.00000000f, -1.96597588f, 0.96748119f, 0, 0, 0, 0}
    };

    // Filter left channel in-place
    bandpass_filter(left_clean, size, stages, FILTER_STAGES);

    // Reset filter states for right channel filtering
    for (int s = 0; s < FILTER_STAGES; s++) {
        stages[s].x1 = 0.0f; stages[s].x2 = 0.0f;
        stages[s].y1 = 0.0f; stages[s].y2 = 0.0f;
    }   

    // Filter right channel in-place
    bandpass_filter(right_clean, size, stages, FILTER_STAGES);


    // 5. Correlate
    *out_sample_offset = cross_correlate(left_clean, right_clean, size, max_sample_lag);

    return true;
}