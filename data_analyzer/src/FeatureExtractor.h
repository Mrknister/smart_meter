#ifndef SMART_SCREEN_FEATUREEXTRACTOR_H
#define SMART_SCREEN_FEATUREEXTRACTOR_H


#include "EventFeatures.h"

class FeatureExtractor {
public:
    typedef  EventFeatures::FeatureType FeatureType;

    template<typename DataPointType>  EventFeatures fromEvent(const Event<DataPointType> &event);
    void setConfig(ClassificationConfig config);
private:
    template<typename DataPointType> void
    extractRms(const Event<DataPointType> &event, std::vector<FeatureType> &feature_vec);

private:
    ClassificationConfig classification_config;
};

template<typename DataPointType> EventFeatures
FeatureExtractor::fromEvent(const Event<DataPointType> &event) {
    FeatureType rms = Algorithms::rootMeanSquare(event.event_data.begin(), event.event_data.end());
    std::vector<FeatureType> f_vect;
    extractRms<DataPointType>(event, f_vect);
    return EventFeatures(event.event_meta_data, f_vect);
}


template<typename DataPointType> void
FeatureExtractor::extractRms(const Event<DataPointType> &event, std::vector<FeatureExtractor::FeatureType> &feature_vec
                             ) {
    FeatureType sub_rms = 0;
    unsigned long data_points_per_period = event.event_meta_data.power_meta_data.dataPointsPerPeriod();
    if (event.event_meta_data.power_meta_data.data_points_stored_before_event >= data_points_per_period) {
        sub_rms = Algorithms::rootMeanSquare(event.before_event_begin(),
                                             event.before_event_begin() + data_points_per_period);
    }
    long loop_end = event.event_end() - event.event_begin() - data_points_per_period;
    auto begin = event.event_begin();
    int period_count = 0;
    for (long count = 0; count <= loop_end; count += data_points_per_period) {
        FeatureType rms = Algorithms::rootMeanSquare(begin, event.event_end());
        rms -= sub_rms;
        feature_vec.push_back(rms);
        begin += data_points_per_period;
        if(period_count >= classification_config.number_of_rms) break;
        ++period_count;


    }
}

void FeatureExtractor::setConfig(ClassificationConfig config) {
    this->classification_config = config;

}


#endif //SMART_SCREEN_FEATUREEXTRACTOR_H
