#include "dsp_pipeline.h"

// --- PRIVATE INTERNAL FUNCTIONS ---
// Remember to declare them as static !!!

/**
 * @brief Scans the raw audio buffers to check for a valid sound event.
 * @param[in] left   Pointer to Left channel ADC buffer.
 * @param[in] right  Pointer to Right channel ADC buffer.
 * @param[in] size   Total number of samples in the buffers.
 * @return true if peak-to-peak amplitude exceeds noise floor, false if room is quiet.
 */
static bool check_amplitude_threshold(const uint16_t* left, const uint16_t* right, uint16_t size) {
    // TODO:
    return true; 
}

/**
 * @brief Isolates human speech by filtering out background ambient noise.
 * @param[in]  input   Pointer to the unfiltered raw ADC buffer.
 * @param[out] output  Pointer to the buffer where filtered data will be saved.
 * @param[in]  size    Total number of samples in the buffers.
 */
static void apply_band_pass(const uint16_t* input, uint16_t* output, uint16_t size) {
    // TODO:
}

/**
 * @brief Computes mathematical alignment to find the arrival delay.
 * @param[in] left     Pointer to filtered Left channel buffer.
 * @param[in] right    Pointer to filtered Right channel buffer.
 * @param[in] size     Total number of samples in the buffers.
 * @param[in] max_lag  The physical search window limit.
 * @return The calculated index sample offset (lag value between -max_lag and +max_lag).
 */
static int16_t cross_correlate(const uint16_t* left, const uint16_t* right, uint16_t size, uint16_t max_lag) {
    // TODO:
    return 0;
}


// --- PUBLIC INTERFACE FUNCTION ---

bool DSP_pipeline(int16_t* out_sample_offset, 
                  const uint16_t* left_mic, const uint16_t* right_mic, 
                  const uint16_t size, const uint16_t max_sample_lag) 
{
    // 1. Detect: check if a sound event is present in the raw audio buffers
    if(!check_amplitude_threshold(left_mic, right_mic, size)){
        return false;
    }

    // TODO: Allocate static or stack-allocated arrays here to hold the intermediate filtered audio tokens

    // 2. Filter: apply band-pass filter to isolate human speech frequencies
    // TODO: Call apply_band_pass() twice: once for left channel and once for right channel.

    // 3. Correlate: Process data if a sound event is confirmed
    // TODO: Call cross_correlate() with the filtered buffers and store the returned lag value into '*out_sample_offset'.

    return true;
}