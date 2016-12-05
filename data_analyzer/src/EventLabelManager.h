#ifndef SMART_SCREEN_LABELEDEVENTS_H
#define SMART_SCREEN_LABELEDEVENTS_H


#include "EventFeatures.h"
#include <map>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>
#include <algorithm>

struct DefaultDataPoint;

template<typename DataPointType = DefaultDataPoint> struct EventLabelManager {
    struct LabelTimePair {
        EventMetaData::LabelType label;
        EventMetaData::TimeType time;
    };

    void addEvent(const Event<DataPointType> &event);

    void addLabeledEvent(const Event<DataPointType> &event);

    void addUnlabeledEvent(const Event<DataPointType> &event);

    boost::optional<EventMetaData::LabelType> getEventLabel(const Event<DataPointType> &event) const;

    void loadLabelsFromFile(std::string file_name);


    std::vector<EventFeatures> labeled_events;
    std::vector<EventFeatures> unlabeled_events;
    std::vector<LabelTimePair> labels;

};

template<typename DataPointType> void EventLabelManager<DataPointType>::addEvent(const Event<DataPointType> &event) {
    if (!event.event_meta_data.label.is_initialized()) {
        auto opt_label = this->getEventLabel(event);
    }

    if (event.event_meta_data.label.is_initialized()) {
        addLabeledEvent(event);
    } else {
        addUnlabeledEvent(event);
    }
}

template<typename DataPointType> void
EventLabelManager<DataPointType>::addLabeledEvent(const Event<DataPointType> &event) {
    this->labeled_events.push_back(EventFeatures::fromEvent(event));
}

template<typename DataPointType> void
EventLabelManager<DataPointType>::addUnlabeledEvent(const Event<DataPointType> &event) {
    this->labeled_events.push_back(EventFeatures::fromEvent(event));
}

template<typename DataPointType> boost::optional<EventMetaData::LabelType>
EventLabelManager<DataPointType>::getEventLabel(const Event<DataPointType> &event) const {
    LabelTimePair to_search;
    to_search.time = event.event_meta_data.event_time;

    auto comparison_function = [](LabelTimePair l1, LabelTimePair l2) {
        const EventMetaData::MSDurationType accepted_time_difference(5000);
        return l1.time + accepted_time_difference < l2.time;
    };
    auto iter = std::upper_bound(labels.begin(), labels.end(), to_search, comparison_function);
    if (!comparison_function(to_search,
                             *iter)) { // if the value we have is not smaller than the upper bound, we found a label.
        event.event_meta_data.label.optional(iter->label);
        return true;
    }
    return false;
}

template<typename DataPointType> void EventLabelManager<DataPointType>::loadLabelsFromFile(std::string file_name) {
    std::ifstream file(file_name);
    if (!file.good()) {
        std::cerr << "Failed to open file: " + file_name + "\n";
        return;
    }
    while (true) {
        LabelTimePair l;
        long epoch;
        file >> epoch;
        l.time = boost::posix_time::from_time_t(epoch);
        file.ignore(10, ',');
        file >> l.label;
        file.ignore(50, '\n');
        if (file.good()) {
            labels.push_back(l);
        } else {
            break;
        }
    }
    std::sort(labels.begin(), labels.end(), [](const LabelTimePair &l1, const LabelTimePair &l2) {
        return l1.time < l2.time;
    });

}

#endif //SMART_SCREEN_LABELEDEVENTS_H
