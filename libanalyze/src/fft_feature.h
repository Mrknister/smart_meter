/** @file */

#ifndef _FFT_FEATURE_H_
#define _FFT_FEATURE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <kiss_fft.h>
#include "libanalyze_config.h"

typedef struct {
    kiss_fft_cpx data[DATA_POINTS_PER_FEATURE];
} FastFourierFeature;



/**
 * @brief This function uses the Kiss_FFT library to calculate the diskrete fourier transformation of the provided buffer.
 * @param feature The result will be stored here
 * @param buffer The data that will be analyzed.
 * @param buffer_size The size of the buffer.
 * @param offset The first data point int the buffer.
 * 
 * The fast fourier transform will be applied to DATA_POINTS_PER_FEATURE data points.
 */
int fast_fourier_transform(FastFourierFeature* feature,  float buffer[], int buffer_size, int offset);


#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif
