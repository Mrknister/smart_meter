#ifndef SMART_SCREEN_DATAANALYZER_H
#define SMART_SCREEN_DATAANALYZER_H

#include <vector>
#include <mutex>
#include "Event.h"
#include "Algorithms.h"

struct DefaultDataPoint;

template<typename DataPointType = DefaultDataPoint> class DataAnalyzer {
public:
    typedef float DataFeatureType;


    void pushEvent(const Event<DataPointType> &e);

    void pushEvent(const Event<DataPointType> &&e);

    void analyzeOneEvent(const Event<DataPointType> &e);

private:

    std::vector<DataAnalyzer::DataFeatureType> extractFeatureVector(const std::vector<DataPointType> eventData);

    std::mutex events_mutex;
    std::vector<Event<DataPointType>> events;
};



template<typename DataPointType> void DataAnalyzer<DataPointType>::pushEvent(const Event<DataPointType> &e) {
    std::lock_guard<std::mutex> events_lock(events_mutex);
    events.push_back(e);
}

template<typename DataPointType> void DataAnalyzer<DataPointType>::pushEvent(const Event<DataPointType> &&e) {
    std::lock_guard<std::mutex> events_lock(events_mutex);
    events.push_back(std::move(e));
}

template<typename DataPointType> void DataAnalyzer<DataPointType>::analyzeOneEvent(const Event<DataPointType> &e) {
    std::vector<DataFeatureType> features = extractFeatureVector(e.event_data);

}

template<typename DataPointType>
std::vector<float>
DataAnalyzer<DataPointType>::extractFeatureVector(const std::vector<DataPointType> eventData) {
    DataFeatureType rms = Algorithms::rootMeanSquare(eventData.begin(), eventData.end());
    return std::vector<DataFeatureType>{rms};
}

#endif //SMART_SCREEN_DATAANALYZER_H




























