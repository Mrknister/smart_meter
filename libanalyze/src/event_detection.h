/** @file */

#ifndef _EVENT_DETECTION_H_
#define _EVENT_DETECTION_H_



#if defined(__cplusplus)
extern "C" {
#endif

#include "libanalyze_config.h"
#include "root_mean_square_feature.h"


/**
 * @brief This function calculates the amplitude for one period. 
 */
float establish_current_amplitude(float* buffer, int buffer_size, int offset);


/**
 * @brief This function detects current inrush by applying the root mean square algorithm onto each amplitude and compares it with the previous rms results.
 * @param voltage A ring buffer of buffer_size length containing voltage information in volts.
 * @param current A ring buffer of buffer_size length containing current information in ampere.
 * @param buffer_size The size of the voltage and current buffers.
 * @param data_points_to_analyze The number of data_points that will be analyzed.
 * @param offset The first data point int the buffer that should be analyzed.
 * 
 * If you wish to lower the threshold you can do that by passing the CURRENT_DIFFERENCE_EVENT_TRIGGER argument to cmake.
 * 
 * Example: 
 * 
 * cmake .. -DCURRENT_DIFFERENCE_EVENT_TRIGGER=0.1
 * 
 */
int detect_event(float* voltage, float* current, int buffer_size, int data_points_to_analyze, int offset);



#if defined(__cplusplus)
} /* extern "C" */
#endif


#endif // ANALYZE_H
