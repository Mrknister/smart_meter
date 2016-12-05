#ifndef EVENTMETADATA_H
#define EVENTMETADATA_H

#include <boost/date_time.hpp>
#include <boost/optional.hpp>
#include "DynamicStreamMetaData.h"

struct EventMetaData {
    typedef DynamicStreamMetaData::TimeType TimeType;

    typedef DynamicStreamMetaData::MSDurationType MSDurationType;
    typedef DynamicStreamMetaData::USDurationType USDurationType;


    TimeType event_time;
    typedef double LabelType;
    unsigned long event_id;
    boost::optional<LabelType> label;
    unsigned long data_points_stored_before_event;
    unsigned long data_points_stored_of_event;

    EventMetaData() {}

    EventMetaData(TimeType time, unsigned long data_points_stored_before_event_,
                  unsigned long data_points_stored_of_event_) : event_time(time), data_points_stored_before_event(
            data_points_stored_before_event_), data_points_stored_of_event(data_points_stored_of_event_){}
};

#endif // EVENTMETADATA_H
