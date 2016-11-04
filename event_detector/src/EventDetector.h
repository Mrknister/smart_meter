#ifndef EVENTDETECTOR_H
#define EVENTDETECTOR_H

#include "DataManager.h"
#include <string>
#include <memory>
#include <thread>
#include "EventMetaData.h"
#include "EventStorage.h"



class EventDetector
{
public:
    void startAnalyzing(DefaultDataManager *data_manager, PowerMetaData meta_data, EventMetaData::TimeType time = boost::posix_time::second_clock::universal_time());

private:
    void run();
    bool readBuffer(DefaultDataPoint* data_point);
    bool detectEvent(DefaultDataPoint *period);
    void storeEvent(DefaultDataPoint *prev_period, DefaultDataPoint *current_period);
    boost::posix_time::time_duration getTimePassed();

    EventMetaData::TimeType getCurrentTime();
private:
    PowerMetaData power_meta_data;
    DefaultDataManager *data_manager;

    EventMetaData::TimeType start_time;
    unsigned long periods_read = 0;

    std::unique_ptr<DefaultDataPoint> electrical_period_buffer1;
    std::unique_ptr<DefaultDataPoint> electrical_period_buffer2;

    EventStorage storage;

    std::thread runner;
};

#endif // EVENTDETECTOR_H
