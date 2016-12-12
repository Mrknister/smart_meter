#ifndef SMART_SCREEN_EVENTFEATURES_H
#define SMART_SCREEN_EVENTFEATURES_H

#include <vector>
#include "EventMetaData.h"
#include "Event.h"
#include "Algorithms.h"

class EventFeatures {
public:
    typedef float FeatureType;
    unsigned int event_id;

    EventMetaData event_meta_data;

    std::vector<FeatureType> feature_vector;

    EventFeatures() {}

    EventFeatures(EventMetaData meta_data, std::vector<FeatureType> f_vect) : event_meta_data(meta_data),
                                                                              feature_vector(std::move(f_vect)) {}

    template<typename DataPointType> static EventFeatures fromEvent(const Event<DataPointType> &event);
};

template<typename DataPointType> EventFeatures EventFeatures::fromEvent(const Event<DataPointType> &event) {
    FeatureType rms = Algorithms::rootMeanSquare(event.event_data.begin(), event.event_data.end());
    std::vector<FeatureType> f_vect{rms};
    return EventFeatures(event.event_meta_data, f_vect);
}


#endif //SMART_SCREEN_EVENTFEATURES_H
