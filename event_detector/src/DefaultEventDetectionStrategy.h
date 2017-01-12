#ifndef SMART_SCREEN_EVENTDETECTIONSTRATEGY_H
#define SMART_SCREEN_EVENTDETECTIONSTRATEGY_H

#include "DefaultDataPoint.h"
#include "Algorithms.h"


class DefaultEventDetectionStrategy {
public:
    DefaultEventDetectionStrategy(float detection_threshold = 0.2) {
        this->threshold = detection_threshold;
    }

    template<typename IteratorType> bool
    detectEvent(IteratorType begin, IteratorType end, unsigned int num_data_points_per_period) {
        if (none_detected_yet) {
            previous_rms = Algorithms::rootMeanSquareOfAmpere(begin, end);
            none_detected_yet = false;
            return false;
        }
        float current_rms = Algorithms::rootMeanSquareOfAmpere(begin, end);

        if (current_rms - threshold > previous_rms) {
            none_detected_yet = true;
#ifdef DEBUG_OUTPUT
            std::cout << "Detected an event" << std::endl;
#endif
            return true;
        } else {
            previous_rms = previous_weight * previous_rms + current_weight * current_rms;
            return false;
        }
    }

private:
    bool none_detected_yet = true;
    float previous_rms = none_detected_yet;
    float threshold;
    float previous_weight = 0.4;
    float current_weight = 1.0f - previous_weight;

};


#endif //SMART_SCREEN_EVENTDETECTIONSTRATEGY_H
