#ifndef SMART_SCREEN_LABELEDEVENTS_H
#define SMART_SCREEN_LABELEDEVENTS_H


#include "EventFeatures.h"
#include <map>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>
#include <algorithm>
#include <boost/optional.hpp>

struct DefaultDataPoint;

template<typename DataPointType = DefaultDataPoint> struct EventLabelManager {
    struct LabelTimePair {
        EventMetaData::LabelType label;
        EventMetaData::TimeType time;
    };

    bool findLabelAndAddEvent(const Event<DataPointType> &event);

    void addLabeledEvent(const Event<DataPointType> &event, EventMetaData::LabelType label);

    void addUnlabeledEvent(const Event<DataPointType> &event);

    boost::optional<EventMetaData::LabelType> getEventLabel(const Event<DataPointType> &event) const;

    void loadLabelsFromFile(std::string file_name);


    std::vector<EventFeatures> labeled_events;
    std::vector<EventFeatures> unlabeled_events;
    std::vector<LabelTimePair> labels;

};

template<typename DataPointType> bool
EventLabelManager<DataPointType>::findLabelAndAddEvent(const Event<DataPointType> &event) {
    boost::optional<EventMetaData::LabelType> opt_label = boost::none;
    if (event.event_meta_data.label == boost::none) {
        opt_label = this->getEventLabel(event);
    }

    if (opt_label != boost::none) {
        addLabeledEvent(event, opt_label.value());
        return true;
    }
    return false;
}

template<typename DataPointType> void
EventLabelManager<DataPointType>::addLabeledEvent(const Event<DataPointType> &event, EventMetaData::LabelType label) {
    this->labeled_events.push_back(EventFeatures::fromEvent(event));
    this->labeled_events.back().event_meta_data.label = label;
}

template<typename DataPointType> void
EventLabelManager<DataPointType>::addUnlabeledEvent(const Event<DataPointType> &event) {
    this->labeled_events.push_back(EventFeatures::fromEvent(event));
}

template<typename DataPointType> boost::optional<EventMetaData::LabelType>
EventLabelManager<DataPointType>::getEventLabel(const Event<DataPointType> &event) const {
    if (labels.begin() == labels.end()) {
        std::cout << "No labels loaded!\n";
        return boost::none;
    }
    LabelTimePair to_search;
    to_search.time = event.event_meta_data.event_time;

    auto comparison_function = [](LabelTimePair l1, LabelTimePair l2) {
        const EventMetaData::MSDurationType accepted_time_difference(5000);
        return l1.time + accepted_time_difference < l2.time;
    };

    auto iter = std::lower_bound(labels.begin(), labels.end(), to_search, comparison_function);

    // if the lower bound + margin is not bigger than the event time we found our label.
    if (!comparison_function(to_search, *iter)) {
        std::cout << "found a label: " << iter->label << " " << iter->time << "\n";
        return iter->label;
    }

    std::cout << "found no label\n";
    if (iter != labels.begin()) {
        std::cout << "closest match before event occurrence is: " << (iter - 1)->time <<  "\n";
    }
    if (iter != labels.end()) {
        std::cout << "closest match after event occurrence is: " << iter->time << "\n";
    }

    return boost::none;
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
