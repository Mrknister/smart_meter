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
    PowerMetaData power_meta_data;
    EventMetaData() {}


    EventMetaData(TimeType time,PowerMetaData meta_data) : event_time(time), power_meta_data(meta_data){}

};

#endif // EVENTMETADATA_H
