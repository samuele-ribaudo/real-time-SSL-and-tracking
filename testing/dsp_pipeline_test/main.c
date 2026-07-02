#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "dsp_pipeline.h"

#define SIZE 1024
#define MAX_LAG 50
#define FILTER_STAGES 3

void save_correlation_to_csv(const char* filename, const int16_t* left, const int16_t* right, uint16_t size, int16_t max_lag) {
    FILE *f = fopen(filename, "w");
    if (!f) return;
    
    fprintf(f, "Lag,Correlation\n");
    
    for (int16_t lag = -max_lag; lag <= max_lag; lag++) {
        int64_t current_correlation = 0;
        for (uint16_t i = 0; i < size; i++) {
            int16_t j = i + lag;
            if (j >= 0 && j < size) {
                int32_t left_val = (int32_t)left[i];
                int32_t right_val = (int32_t)right[j];
                current_correlation += left_val * right_val;
            }
        }
        fprintf(f, "%d,%lld\n", lag, (long long)current_correlation);
    }
    
    fclose(f);
}

int main() {
    uint16_t left_raw[SIZE], right_raw[SIZE];
    int16_t left_raw_int[SIZE], right_raw_int[SIZE];
    int16_t left_filtered[SIZE], right_filtered[SIZE];

    FILE *f_in = fopen("files/sample_1_raw.csv", "r");
    if (!f_in) {
        printf("Error: Could not open input file.\n");
        return 1;
    }

    char buffer[128];
    fgets(buffer, sizeof(buffer), f_in); 

    for (int i = 0; i < SIZE && fgets(buffer, sizeof(buffer), f_in); i++) {
        sscanf(buffer, "%hu,%hu", &left_raw[i], &right_raw[i]);
    }
    fclose(f_in);

    remove_dc_offset(left_raw, left_raw_int, SIZE);
    remove_dc_offset(right_raw, right_raw_int, SIZE);

    // Copy DC-centered data to filtered arrays for in-place filtering
    for(int i = 0; i < SIZE; i++) {
        left_filtered[i] = left_raw_int[i];
        right_filtered[i] = right_raw_int[i];
    }

    /*
    --- Cascaded Biquad Coefficients (SOS) ---
    .b0 = 0.00357387, .b1 = 0.00714774, .b2 = 0.00357387, .a1 = -1.69527851, .a2 = 0.70751554
    .b0 = 1.00000000, .b1 = 0.00000000, .b2 = -1.00000000, .a1 = -1.62501451, .a2 = 0.73868441
    .b0 = 1.00000000, .b1 = -2.00000000, .b2 = 1.00000000, .a1 = -1.96597588, .a2 = 0.96748119
    */

    // Initialize unified bandpass filter stages
    biquad_window stages[FILTER_STAGES] = {
        {0.00357387f, 0.00714774f, 0.00357387f, -1.69527851f, 0.70751554f, 0, 0, 0, 0},
        {1.00000000f, 0.00000000f, -1.00000000f, -1.62501451f, 0.73868441f, 0, 0, 0, 0},
        {1.00000000f, -2.00000000f, 1.00000000f, -1.96597588f, 0.96748119f, 0, 0, 0, 0}
    };

    // Filter Left Channel IN-PLACE
    bandpass_filter(left_filtered, SIZE, stages, FILTER_STAGES);

    // Reset filter states for right channel filtering
    for (int s = 0; s < FILTER_STAGES; s++) {
        stages[s].x1 = 0.0f; stages[s].x2 = 0.0f;
        stages[s].y1 = 0.0f; stages[s].y2 = 0.0f;
    }   

    // Filter Right Channel IN-PLACE
    bandpass_filter(right_filtered, SIZE, stages, FILTER_STAGES);

    FILE *f_out = fopen("files/sample_1_filtered.csv", "w");
    if (f_out) {
        fprintf(f_out, "Left_Channel_Filtered,Right_Channel_Filtered\n");
        for (int i = 0; i < SIZE; i++) {
            fprintf(f_out, "%d,%d\n", left_filtered[i], right_filtered[i]);
        }
        fclose(f_out);
    }

    save_correlation_to_csv("files/correlation_raw.csv", left_raw_int, right_raw_int, SIZE, MAX_LAG);
    save_correlation_to_csv("files/correlation_filtered.csv", left_filtered, right_filtered, SIZE, MAX_LAG);

    int16_t lag_raw = cross_correlate(left_raw_int, right_raw_int, SIZE, MAX_LAG);
    int16_t lag_filtered = cross_correlate(left_filtered, right_filtered, SIZE, MAX_LAG);

    printf("Raw Data Lag (DC Removed): %d\n", lag_raw);
    printf("Filtered Data Lag: %d\n", lag_filtered);

    return 0;
}