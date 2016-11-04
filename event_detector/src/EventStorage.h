#ifndef EVENTSTORAGE_H
#define EVENTSTORAGE_H

#include <vector>
#include <string>
#include <fstream>

#include "DefaultDataPoint.h"
#include "EventMetaData.h"

class EventStorage
{
public:
    template <typename IteratorType> unsigned int storeEvent(IteratorType begin, const IteratorType end, const EventMetaData& meta_data);


private:
    template <typename IteratorType>
    void writeToFile(IteratorType begin, const IteratorType end, const std::string& file_name);
};


template<typename IteratorType>
unsigned int EventStorage::storeEvent(IteratorType begin, const IteratorType end, const EventMetaData&  meta_data) {
    static unsigned int uuid = 0;
    writeToFile(begin, end, "event_" + std::to_string(uuid) + "_" + boost::posix_time::to_simple_string(meta_data.event_time));
    return uuid++;
}

template<typename IteratorType> void
EventStorage::writeToFile(IteratorType begin, const IteratorType end, const std::string &file_name) {
    std::ofstream out_stream(file_name);
    while(begin != end) {
        out_stream << begin->volts << "," << begin->ampere << "\n";
        ++begin;
    }
}

#endif // EVENTSTORAGE_H
