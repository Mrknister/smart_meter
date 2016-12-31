#ifndef EVENTSTORAGE_H
#define EVENTSTORAGE_H

#include <vector>
#include <string>
#include <fstream>
#include <functional>


#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "DefaultDataPoint.h"
#include "EventMetaData.h"
#include "Event.h"


template<typename DataPointType=DefaultDataPoint> class EventStorage {
public:
    template<typename IteratorType> unsigned long
    storeEvent(IteratorType begin, const IteratorType end, const EventMetaData &meta_data);

    template<typename IteratorType> void
    storeFeatureVector(IteratorType begin, const IteratorType end, unsigned long event_uuid);

    Event<DataPointType> loadEvent(unsigned long event_uuid);

    void setEventStorageCallback(std::function<void(Event<DataPointType> &)> callBack);


private:
    std::function<void(Event<DataPointType> &)> callback = [](Event<DataPointType> &) {};

    template<typename IteratorType> void
    writeToFile(IteratorType begin, const IteratorType end, const std::string &file_name,
                const EventMetaData &meta_data, const unsigned long id);

    template<typename IteratorType> void
    writeToCSV(IteratorType begin, const IteratorType end, const std::string &file_name);

    template<typename IteratorType> std::vector<DataPointType>
    eventDataToVector(IteratorType begin, const IteratorType end);

    void storeEventDataToCSV(const Event<DataPointType> &to_store);

    std::string createFilePath(unsigned long uuid);

public:
    std::string event_directory = "events/";
private:
    enum {
        buffer_size = 256
    };
};

template<typename DataPointType> template<typename IteratorType> unsigned long
EventStorage<DataPointType>::storeEvent(IteratorType begin, const IteratorType end, const EventMetaData &meta_data) {
    static unsigned long uuid = 0;
    writeToFile<IteratorType>(begin, end, createFilePath(uuid), meta_data, uuid);
    return uuid++;
}

template<typename DataPointType> template<typename IteratorType> void
EventStorage<DataPointType>::writeToFile(IteratorType begin, const IteratorType end, const std::string &file_name,
                                         const EventMetaData &meta_data, const unsigned long id) {
    std::ofstream out_stream(file_name);
    boost::archive::text_oarchive oa(out_stream);
    // write class instance to archive
    Event<DataPointType> event;

    event.event_data = eventDataToVector<IteratorType>(begin, end);
    event.event_meta_data = meta_data;
    event.event_meta_data.event_id = id;

    this->storeEventDataToCSV(event);

    this->callback(event);

    oa << event;
    out_stream.close();
}

template<typename DataPointType> Event<DataPointType> EventStorage<DataPointType>::loadEvent(unsigned long event_uuid) {
    std::string file_path = createFilePath(event_uuid);

    std::ifstream ifs(file_path);
    boost::archive::text_iarchive ia(ifs);


    Event<DataPointType> result;
    ia >> result;
    return result;
}

template<typename DataPointType>

template<typename IteratorType> std::vector<DataPointType>
EventStorage<DataPointType>::eventDataToVector(IteratorType begin, const IteratorType end) {
    return std::vector<DataPointType>(begin, end);
}

template<typename DataPointType> std::string EventStorage<DataPointType>::createFilePath(unsigned long uuid) {
    return event_directory + "/event_" + std::to_string(uuid) + ".archive";
}

template<typename DataPointType> void
EventStorage<DataPointType>::setEventStorageCallback(std::function<void(Event<DataPointType> &)> callBack) {
    this->callback = callBack;
}

template<typename DataPointType> void
EventStorage<DataPointType>::storeEventDataToCSV(const Event<DataPointType> &to_store) {
    std::string file_path(event_directory + "/event_" + std::to_string(to_store.event_meta_data.event_id) + ".csv");
    std::ofstream file_stream(file_path);
    if (!file_stream.good()) {
        std::cerr << "Could not open path: " << file_path << std::endl;
        throw std::exception();
    }
    for (auto &data_point: to_store.event_data) {
        file_stream << data_point.voltage() << "," << data_point.ampere() << "\n";
    }
}


template<typename DataPointType> template<typename IteratorType> void
EventStorage<DataPointType>::storeFeatureVector(IteratorType begin, const IteratorType end, unsigned long event_uuid) {
    std::string file_name = event_directory + "/event_" + std::to_string(event_uuid) + "_feature_vec.csv";
    writeToCSV(begin,end, file_name);
}

template<typename DataPointType> template<typename IteratorType> void
EventStorage<DataPointType>::writeToCSV(IteratorType begin, const IteratorType end, const std::string &file_name) {
    std::ofstream file_stream(file_name);
    if (!file_stream.good()) {
        std::cerr << "Could not open path: " << file_name << std::endl;
        throw std::exception();
    }
    while(begin!=end) {
        file_stream << *begin << "\n";
        ++begin;
    }
}

#endif // EVENTSTORAGE_H
