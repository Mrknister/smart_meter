#ifndef SMART_SCREEN_EVENTDETECTIONSTRATEGY_H
#define SMART_SCREEN_EVENTDETECTIONSTRATEGY_H

#include "DefaultDataPoint.h"
#include "Algorithms.h"


class DefaultEventDetectionStrategy {
public:
    DefaultEventDetectionStrategy(float detection_threshold = 1.0){
        this->threshold = detection_threshold;
    }

    bool detectEvent(DefaultDataPoint * previous_period, DefaultDataPoint* current_period, unsigned int num_data_points_per_period) {
        if(previous_rms == -100000) {
            previous_rms = Algorithms::rootMeanSquare(previous_period, previous_period + num_data_points_per_period);
        }
        float current_rms = Algorithms::rootMeanSquare(current_period, current_period + num_data_points_per_period);
        bool success = current_rms - threshold > previous_rms;
        previous_rms = current_rms;
        return success;
    }

private:
    float previous_rms = -100000;
    float threshold;

};


#endif //SMART_SCREEN_EVENTDETECTIONSTRATEGY_H
