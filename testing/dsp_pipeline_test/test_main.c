#include <stdio.h>
#include <stdbool.h>
#include "dsp_pipeline.h"

int main() {
    // 1. Create fake 20-sample audio buffers. 
    // Notice the "peak" of 4000 happens at index 4 on the left, and index 7 on the right.
    // This means the right ear heard it 3 samples later!
    uint16_t left_ear[20]  = {3200, 3200, 3200, 3800, 4000, 3800, 3200, 3200, 3200, 3200, 3200, 3200, 3200, 3200, 3200, 3200, 3200, 3200, 3200, 3200};
    uint16_t right_ear[20] = {3200, 3200, 3200, 3200, 3200, 3200, 3800, 4000, 3800, 3200, 3200, 3200, 3200, 3200, 3200, 3200, 3200, 3200, 3200, 3200};
    
    int16_t calculated_offset = 0;
    uint16_t threshold = 200; // The threshold we established

    // 2. Run your pipeline!
    bool sound_detected = DSP_pipeline(&calculated_offset, threshold, left_ear, right_ear, 20, 10);

    // 3. Print the results to your terminal
    if (sound_detected) {
        printf("SUCCESS! Sound detected.\n");
        printf("Calculated lag: %d samples\n", calculated_offset);
        if (calculated_offset == 3) {
            printf("MATCH! The cross-correlation math is working perfectly.\n");
        } else {
            printf("ERROR: Math failed. Expected 3.\n");
        }
    } else {
        printf("FAILED: Pipeline thought the room was quiet.\n");
    }

    return 0;
}
