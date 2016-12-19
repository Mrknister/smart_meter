#ifndef SMART_SCREEN_LABELEDEVENTS_H
#define SMART_SCREEN_LABELEDEVENTS_H


#include "EventFeatures.h"
#include <map>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>
#include <algorithm>
#include <boost/optional.hpp>
#include <set>


struct DefaultDataPoint;

struct LabelTimePair {
    EventMetaData::LabelType label;
    EventMetaData::TimeType time;
};
namespace __detail {
    struct TimePairComparator {
        static bool compareLabels(const LabelTimePair &l1, const LabelTimePair &l2) {
            const EventMetaData::MSDurationType accepted_time_difference(5000);
            return l1.time + accepted_time_difference < l2.time;
        }

        bool operator()(const LabelTimePair &l1, const LabelTimePair &l2) const {
            return compareLabels(l1, l2);
        };
    };
}
template<typename DataPointType = DefaultDataPoint>
class EventLabelManager {

public:
    bool findLabelAndAddEvent(const EventFeatures &event);

    void addLabeledEvent(const EventFeatures &event, EventMetaData::LabelType label);

    void addClassifiedEvent(const EventFeatures &event);

    bool addLabel(const LabelTimePair labels);

    boost::optional<EventMetaData::LabelType> getEventLabel(const EventFeatures &event) const;

    bool labelIsForEvent(const LabelTimePair &label_time, const EventFeatures &event);

    void loadLabelsFromFile(std::string file_name);


    std::vector<EventFeatures> labeled_events;
    std::vector<EventFeatures> unlabeled_events;
    std::set<LabelTimePair, __detail::TimePairComparator> labels;

private:


};

template<typename DataPointType>
bool
EventLabelManager<DataPointType>::findLabelAndAddEvent(const EventFeatures &event) {
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

template<typename DataPointType>
void
EventLabelManager<DataPointType>::addLabeledEvent(const EventFeatures &event, EventMetaData::LabelType label) {
    this->labeled_events.push_back(event);
    this->labeled_events.back().event_meta_data.label = label;
}

template<typename DataPointType>
void EventLabelManager<DataPointType>::addClassifiedEvent(const EventFeatures &event) {
    this->labeled_events.push_back(event);
}

template<typename DataPointType>
boost::optional<EventMetaData::LabelType>
EventLabelManager<DataPointType>::getEventLabel(const EventFeatures &event) const {
    if (labels.empty()) {
        std::cout << "No labels loaded!\n";
        return boost::none;
    }
    LabelTimePair to_search;
    to_search.time = event.event_meta_data.event_time;
    auto iter = labels.find(to_search);
    if (iter != this->labels.end()) {
        return iter->label;
    }

    std::cout << "found no label\n";
    iter = labels.lower_bound(to_search);
    if (iter != labels.end()) {
        std::cout << "closest match after event occurrence is: " << iter->time << "\n";
    }
    if (iter != labels.begin()) {
        --iter;
        std::cout << "closest match before event occurrence is: " << iter->time << "\n";
    }

    return boost::none;
}

template<typename DataPointType>
void EventLabelManager<DataPointType>::loadLabelsFromFile(std::string file_name) {
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
            labels.insert(l);
        } else {
            break;
        }
    }
}

template<typename DataPointType>
bool EventLabelManager<DataPointType>::labelIsForEvent(const LabelTimePair &label_time, const EventFeatures &event) {
    LabelTimePair to_search;
    to_search.time = event.event_meta_data.event_time;
    return !__detail::TimePairComparator::compareLabels(to_search, label_time) &&
           !__detail::TimePairComparator::compareLabels(label_time, to_search);
}

#endif //SMART_SCREEN_LABELEDEVENTS_H
