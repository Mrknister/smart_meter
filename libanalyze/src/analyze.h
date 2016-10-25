/**
 * @file 
 * 
 * This library contains some algorithms to analyze high frequency energy data. Data passed to functions in this library is expected to be in a ring buffer. 
 * You will always have to provide a buffer, its length and an offset at which the relevant data starts. 
 * 
 */


#ifndef _ANALYZE_H_
#define _ANALYZE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "event_detection.h"
#include "fft_feature.h"
#include "root_mean_square_feature.h"

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif
