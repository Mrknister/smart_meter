#ifndef EVENTDETECTOR_H
#define EVENTDETECTOR_H

#include "DataManager.h"
#include <string>
#include <memory>
#include <thread>
#include "EventMetaData.h"
#include "EventStorage.h"
#include <utility>
#include "DefaultEventDetectionStrategy.h"


template<typename EventDetectionStrategyType = DefaultEventDetectionStrategy> class EventDetector {
public:
    /**
     * @brief This function spawns a thread that reads data from a DefaultDataManager and detects events in it. If the thread is already running the program will wait until it has ended.
     * @param input_data_manager
     * @param meta_data
     * @param time
     */
    void
    startAnalyzing(DefaultDataManager *input_data_manager, PowerMetaData meta_data, EventDetectionStrategyType strategy,
                   EventMetaData::TimeType time = boost::posix_time::second_clock::universal_time());

    /**
     * @brief Waits until a full period can be read and analyzes it for events. If one is detected it reads additional data until the event can be stored
     */
    void stopGracefully();

    /**
     * @brief Stops the thread as fast as possible, possibly disarding data.
     */
    void stopNow();


private:
    void run();

    bool readBuffer(DefaultDataPoint *data_point);

    bool detectEvent(DefaultDataPoint *prev_period, DefaultDataPoint *current_period);

    void storeEvent(DefaultDataPoint *prev_period, DefaultDataPoint *current_period);

    boost::posix_time::time_duration getTimePassed();

    EventMetaData::TimeType getCurrentTime();

private:
    bool continue_analyzing = true;
    bool stop_now = false;

    EventDetectionStrategyType event_detection_strategy;
    PowerMetaData power_meta_data;
    DefaultDataManager *data_manager;

    EventMetaData::TimeType stream_start_time;
    unsigned long periods_read = 0;

    std::unique_ptr<DefaultDataPoint> electrical_period_buffer1;
    std::unique_ptr<DefaultDataPoint> electrical_period_buffer2;

    EventStorage storage;

    std::thread runner;
};

template<class EventDetectionStrategyType> void
EventDetector<EventDetectionStrategyType>::startAnalyzing(DefaultDataManager *input_data_manager,
                                                          PowerMetaData meta_data, EventDetectionStrategyType strategy,
                                                          EventMetaData::TimeType start_time) {
    // block until the thread has ended. We can't have 2 threads writing into the buffers at the same time.
    if (this->runner.joinable()) {
        this->runner.join();
    }
    this->continue_analyzing = true;
    this->stop_now = false;
    this->data_manager = input_data_manager;
    this->power_meta_data = meta_data;
    this->event_detection_strategy = std::move(strategy);


    this->stream_start_time = start_time;


    // create buffers for the electrical periods
    this->electrical_period_buffer1 = std::unique_ptr<DefaultDataPoint>(
            new DefaultDataPoint[meta_data.dataPointsPerPeriod()]);
    this->electrical_period_buffer2 = std::unique_ptr<DefaultDataPoint>(
            new DefaultDataPoint[meta_data.dataPointsPerPeriod()]);

    runner = std::thread(&EventDetector<EventDetectionStrategyType>::run, this);
}

template<class EventDetectionStrategyType> void EventDetector<EventDetectionStrategyType>::run() {
    DefaultDataPoint *buffer_prev_period = this->electrical_period_buffer1.get();
    DefaultDataPoint *buffer_current_period = this->electrical_period_buffer2.get();
    this->readBuffer(buffer_prev_period);
    while (this->readBuffer(buffer_current_period) && this->continue_analyzing) {
        if (this->detectEvent(buffer_prev_period, buffer_current_period)) {
            this->storeEvent(buffer_prev_period, buffer_current_period);
        }
        std::swap(buffer_prev_period, buffer_current_period);
    }
}

template<class EventDetectionStrategyType> bool
EventDetector<EventDetectionStrategyType>::readBuffer(DefaultDataPoint *data_point) {
    DefaultDataPoint *buffer_end = data_point + this->power_meta_data.dataPointsPerPeriod();
    DefaultDataPoint *data_end = data_manager->popDataPoints(data_point, buffer_end);
    this->periods_read += 1;
    return data_end == buffer_end;
}

template<class EventDetectionStrategyType> bool
EventDetector<EventDetectionStrategyType>::detectEvent(DefaultDataPoint *prev_period,
                                                       DefaultDataPoint *current_period) {
    if (this->event_detection_strategy.detectEvent(prev_period, current_period, this->power_meta_data.dataPointsPerPeriod())) {
        std::cout << "time: " << this->getCurrentTime() << " = " << this->stream_start_time << " + "
                  << this->getTimePassed().total_milliseconds() / 1000.f << std::endl;

        return true;

    }
    return false;
}

template<class EventDetectionStrategyType> void
EventDetector<EventDetectionStrategyType>::storeEvent(DefaultDataPoint *prev_period, DefaultDataPoint *current_period) {
    // Make sure we want to store at least one period of data
    if (this->power_meta_data.periods_stored <= 0 || this->stop_now) { return; }

    // store the two periods we already accuired in a buffer_vector
    std::vector<DefaultDataPoint> to_store(prev_period, prev_period + this->power_meta_data.dataPointsPerPeriod());
    to_store.insert(to_store.end(), current_period, current_period + this->power_meta_data.dataPointsPerPeriod());
    to_store.resize((this->power_meta_data.periods_stored + 1) * this->power_meta_data.dataPointsPerPeriod());
    //get the rest of the periods we want
    this->data_manager->popDataPoints(to_store.begin() + this->power_meta_data.dataPointsPerPeriod() * 2,
                                      to_store.end());
    this->storage.storeEvent(to_store.begin(), to_store.end(), EventMetaData(this->getCurrentTime()));

}

template<class EventDetectionStrategyType> boost::posix_time::time_duration
EventDetector<EventDetectionStrategyType>::getTimePassed() {
    return boost::posix_time::seconds(periods_read) / this->power_meta_data.frequency;
}

template<class EventDetectionStrategyType> EventMetaData::TimeType
EventDetector<EventDetectionStrategyType>::getCurrentTime() {
    return this->stream_start_time + this->getTimePassed();
}

template<class EventDetectionStrategyType> void EventDetector<EventDetectionStrategyType>::stopGracefully() {
    this->continue_analyzing = false;
    this->runner.join();
}

template<class EventDetectionStrategyType> void EventDetector<EventDetectionStrategyType>::stopNow() {
    this->continue_analyzing = false;
    this->stop_now = true;

    this->data_manager->unblockReadOperations();
    this->runner.join();
}

#endif // EVENTDETECTOR_H
