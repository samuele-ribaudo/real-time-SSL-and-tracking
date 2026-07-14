/**
 * @file    dsp_pipeline.h
 * @brief   Public interface for the Digital Signal Processing (DSP) pipeline.
 *
 * @details This header exposes the main DSP function used to calculate the 
 * Time Delay of Arrival (TDOA) between two audio channels. It provides a 
 * clean abstraction, allowing the main control loop to execute the complex 
 * mathematical pipeline (event detection, signal conditioning, filtering, 
 * and cross-correlation) through a single function call.
 */

#ifndef DSP_PIPELINE_H
#define DSP_PIPELINE_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Processes dual-channel audio buffers for TDOA.
 * @param[out] out_sample_offset    Pointer where the calculated signed lag will be stored.
 * @param[in]  quiet_room_treshold  Treshold value to check whether the room is quiet
 * @param[in]  left_mic             Pointer to Left channel ADC buffer.
 * @param[in]  right_mic            Pointer to Right channel ADC buffer.
 * @param[in]  size                 Total number of samples in the buffers.
 * @param[in]  max_sample_lag       The maximum boundary for the cross-correlation search window.
 * @return true if sound was detected and processed, false if no sound is detected.
 */
bool DSP_pipeline(int16_t* out_sample_offset, const uint16_t quiet_room_treshold,
                      const uint16_t* left_mic, const uint16_t* right_mic, 
                      const uint16_t size, const uint16_t max_sample_lag);

#endif // DSP_PIPELINE_H
