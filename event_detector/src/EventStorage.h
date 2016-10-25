#ifndef EVENTSTORAGE_H
#define EVENTSTORAGE_H

#include <vector>
#include "DataPoint.h"
#include "EventMetaData.h"

class EventStorage
{
public:
    void storeEvent(const std::vector<DataPoint> event_data);
    void storeEvent(const std::vector<DataPoint> event_data, EventMetaData meta_data);

};

#endif // EVENTSTORAGE_H
