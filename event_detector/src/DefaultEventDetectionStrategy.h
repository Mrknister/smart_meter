#ifndef SMART_SCREEN_EVENTDETECTIONSTRATEGY_H
#define SMART_SCREEN_EVENTDETECTIONSTRATEGY_H

#include "DefaultDataPoint.h"
#include "Algorithms.h"


class DefaultEventDetectionStrategy {
public:
    DefaultEventDetectionStrategy(float detection_threshold = 0.7) {
        this->threshold = detection_threshold;
    }

    template<typename IteratorType> bool
    detectEvent(IteratorType begin, IteratorType end, unsigned int num_data_points_per_period) {
        if (previous_rms == -100000) {
            previous_rms = Algorithms::rootMeanSquare(begin, end);
            return false;
        }
        float current_rms = Algorithms::rootMeanSquare(begin, end);

        if (current_rms - threshold > previous_rms) {
            previous_rms = current_rms;
            return true;
        } else {
            previous_rms = previous_weight * previous_rms + current_weight * current_rms;
            return false;
        }
    }

private:
    float previous_rms = -100000;
    float threshold;
    float previous_weight = 0.8;
    float current_weight = 1.0 - previous_weight;

};


#endif //SMART_SCREEN_EVENTDETECTIONSTRATEGY_H
