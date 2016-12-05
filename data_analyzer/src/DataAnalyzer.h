#ifndef SMART_SCREEN_DATAANALYZER_H
#define SMART_SCREEN_DATAANALYZER_H

#include <vector>
#include <mutex>
#include <condition_variable>
#include <dlib/graph_utils.h>
#include "EventFeatures.h"
#include "Algorithms.h"
#include "EventLabelManager.h"


struct DefaultDataPoint;

template<typename DataPointType = DefaultDataPoint> class DataAnalyzer {
public:
    typedef float DataFeatureType;

    void startAnalyzing();

    void pushEvent(const Event<DataPointType> &e);

    void analyzeOneEvent(const Event<DataPointType> &e);

private:
    static float calculateDistance(const EventFeatures &e1, const EventFeatures &e2);


public:
    EventLabelManager<DataPointType> event_label_manager;
private:
    std::thread runner;
    bool continue_analyzing = true;
    std::mutex events_mutex;
    std::condition_variable events_empty_variable;
    std::vector<EventFeatures> events;

};


template<typename DataPointType> void DataAnalyzer<DataPointType>::pushEvent(const Event<DataPointType> &e) {
    std::lock_guard<std::mutex> events_lock(events_mutex);
    events.push_back(EventFeatures::fromEvent<DataPointType>(e));
}


template<typename DataPointType> void DataAnalyzer<DataPointType>::analyzeOneEvent(const Event<DataPointType> &e) {
    const unsigned long k = 5;
    std::vector<dlib::sample_pair> out;
    auto distance_function = std::bind(&DataAnalyzer<DataPointType>::calculateDistance, this);
    dlib::find_k_nearest_neighbors(this->event_label_manager.labeled_events, distance_function, k, out);
}


template<typename DataPointType> float
DataAnalyzer<DataPointType>::calculateDistance(const EventFeatures &e1, const EventFeatures &e2) {
    return Algorithms::distance(e1.feature_vector.begin(), e1.feature_vector.end(), e2.feature_vector.begin(),
                                e2.feature_vector.end());
}

void DataAnalyzer::startAnalyzing() {
    runner = std::thread([this]() {
        while(this->continue_analyzing) {

        }
    });

}


#endif //SMART_SCREEN_DATAANALYZER_H




























