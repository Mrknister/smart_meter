/** @file */
#ifndef _FEATURES_H_
#define _FEATURES_H_

#include "libanalyze_config.h"

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct {
    float data[ANALYZED_FEATURES_PER_WAVE];
} RootMeanSquareFeature;


/**
 * @brief Calculates to root mean square of the current. Current is expected to be a ring buffer.
 * @param voltage A ring buffer of buffer_size length containing voltage information in volts.
 * @param current A ring buffer of buffer_size length containing current information in ampere.
 * @param buffer_size The size of the voltage and current buffers.
 * @param offset The first data point int the buffer that should be analyzed.
 * 
 * This funtion calculates the rms for one period.
 */
float root_mean_square(float voltage[], float current[], int buffer_size, int offset);


/**
 * @brief Applies root_mean_square to DATA_POINTS_PER_FEATURE data points.
 */
int root_mean_square_feature(RootMeanSquareFeature* feature, float voltage[], float current[], int buffer_size, int offset);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif

