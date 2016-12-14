#ifndef SMART_SCREEN_EVENTFEATURES_H
#define SMART_SCREEN_EVENTFEATURES_H

#include <vector>
#include "EventMetaData.h"
#include "Event.h"
#include "Algorithms.h"
#include "ClassificationConfig.h"

class EventFeatures {
public:
    typedef float FeatureType;
    unsigned int event_id;

    EventMetaData event_meta_data;

    std::vector<FeatureType> feature_vector;

    EventFeatures() {}

    EventFeatures(EventMetaData meta_data, std::vector<FeatureType> f_vect) : event_meta_data(meta_data),
                                                                              feature_vector(std::move(f_vect)) {}


};




#endif //SMART_SCREEN_EVENTFEATURES_H
