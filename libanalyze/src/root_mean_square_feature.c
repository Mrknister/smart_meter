

#include "root_mean_square_feature.h"
#include <math.h>

#if defined(__cplusplus)
extern "C" {
#endif


float root_mean_square(float voltage[], float current[], int buffer_size, int offset)
{
    
    float result = 0;
    int byte_counter = 0;
    for (; byte_counter < DATA_POINTS_PER_WAVE_LENGTH; ++ byte_counter) {
        result += current[(byte_counter + offset) % buffer_size] * current[(byte_counter + offset) % buffer_size];
    }
    
    return sqrt(result / DATA_POINTS_PER_WAVE_LENGTH);
}

int root_mean_square_feature(RootMeanSquareFeature* feature, float voltage[], float current[], int buffer_size, int offset)
{

    int msq_counter = 0;
    for (; msq_counter < ANALYZED_FEATURES_PER_WAVE; msq_counter++) {
        feature->data[msq_counter] = 0;
    }

    msq_counter = 0;

    int bytes_counter = 0;
    while (bytes_counter < DATA_POINTS_PER_FEATURE - DATA_POINTS_PER_WAVE_LENGTH) {

        feature->data[msq_counter] = root_mean_square(voltage, current, buffer_size, bytes_counter + offset);
        bytes_counter += DATA_POINTS_PER_WAVE_LENGTH;
        ++msq_counter;

       
    }
    return 0;
}

#if defined(__cplusplus)
} /* extern "C" */
#endif

