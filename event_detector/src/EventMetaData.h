#ifndef EVENTMETADATA_H
#define EVENTMETADATA_H

#include <boost/date_time.hpp>

struct EventMetaData
{
    typedef boost::posix_time::ptime TimeType;
    TimeType event_time;
    EventMetaData(){};
    EventMetaData( TimeType time):event_time(time){}
};

#endif // EVENTMETADATA_H
