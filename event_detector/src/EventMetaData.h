#ifndef EVENTMETADATA_H
#define EVENTMETADATA_H

#include <boost/date_time.hpp>
#include "DynamicStreamMetaData.h"

struct EventMetaData
{
    typedef DynamicStreamMetaData::TimeType TimeType;
    TimeType event_time;
    EventMetaData(){};
    EventMetaData( TimeType time):event_time(time){}
};

#endif // EVENTMETADATA_H
