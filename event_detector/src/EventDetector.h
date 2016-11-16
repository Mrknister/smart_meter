#ifndef EVENTDETECTOR_H
#define EVENTDETECTOR_H

#include "DataManager.h"
#include <string>
#include <memory>
#include <thread>
#include <cassert>
#include "EventMetaData.h"
#include "EventStorage.h"
#include <utility>
#include "DefaultEventDetectionStrategy.h"


template<typename EventDetectionStrategyType = DefaultEventDetectionStrategy, typename DataPointType = DefaultDataPoint> class EventDetector {
public:
    /**
     * @brief This function spawns a thread that reads data from a DefaultDataManager and detects events in it. If the thread is already running the program will wait until it has ended.
     * @param input_data_manager
     * @param meta_data
     * @param time
     */
    void startAnalyzing(DefaultDataManager *input_data_manager, DynamicStreamMetaData *meta_data,
                        EventDetectionStrategyType strategy = EventDetectionStrategyType());

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

    bool readBuffer(DataPointType *data_point);

    bool detectEvent(DataPointType *prev_period, DataPointType *current_period);

    void storeEvent(DataPointType *prev_period, DataPointType *current_period);

private:
    bool continue_analyzing = true;
    bool stop_now = false;

    EventDetectionStrategyType event_detection_strategy;
    DynamicStreamMetaData *dynamic_meta_data;

    PowerMetaData power_meta_data;
    DefaultDataManager *data_manager;
    DynamicStreamMetaData::DataPointIdType data_points_read = -1;
    unsigned long buffer_length;
    std::unique_ptr<DataPointType[]> electrical_period_buffer1;
    std::unique_ptr<DataPointType[]> electrical_period_buffer2;

    EventStorage storage;

    std::thread runner;
};

template<typename EventDetectionStrategyType, typename DataPointType> void
EventDetector<EventDetectionStrategyType, DataPointType>::startAnalyzing(DefaultDataManager *input_data_manager,
                                                                         DynamicStreamMetaData *meta_data,
                                                                         EventDetectionStrategyType strategy) {
    assert(input_data_manager != nullptr);
    assert(meta_data != nullptr);

    // block until the thread has ended. We can't have 2 threads writing into the buffers at the same time.
    if (this->runner.joinable()) {
        this->runner.join();
    }
    this->continue_analyzing = true;
    this->stop_now = false;
    this->data_manager = input_data_manager;
    this->dynamic_meta_data = meta_data;
    this->power_meta_data = meta_data->getFixedPowerMetaData();
    this->event_detection_strategy = std::move(strategy);

    this->buffer_length = power_meta_data.dataPointsPerPeriod();

    // create buffers for the electrical periods
    this->electrical_period_buffer1 = std::unique_ptr<DataPointType[]>(
            new DataPointType[buffer_length]);
    this->electrical_period_buffer2 = std::unique_ptr<DataPointType[]>(
            new DataPointType[buffer_length]);

    runner = std::thread(&EventDetector<EventDetectionStrategyType, DataPointType>::run, this);
}

template<typename EventDetectionStrategyType, typename DataPointType> void
EventDetector<EventDetectionStrategyType, DataPointType>::run() {
    DataPointType *buffer_prev_period = this->electrical_period_buffer1.get();
    DataPointType *buffer_current_period = this->electrical_period_buffer2.get();
    this->readBuffer(buffer_prev_period);
    while (this->readBuffer(buffer_current_period) && this->continue_analyzing) {
        if (this->detectEvent(buffer_prev_period, buffer_current_period)) {
            this->storeEvent(buffer_prev_period, buffer_current_period);
        }
        std::swap(buffer_prev_period, buffer_current_period);
    }
}

template<typename EventDetectionStrategyType, typename DataPointType> bool
EventDetector<EventDetectionStrategyType, DataPointType>::readBuffer(DataPointType *data_point) {
    DataPointType *buffer_end = data_point + this->buffer_length;

    DataPointType *data_end = data_manager->getDataPoints(data_point, buffer_end,
                                                          static_cast<unsigned long> (this->power_meta_data.data_points_stored_before_event));
    data_manager->nextDataPoints(this->buffer_length);
    this->data_points_read += this->buffer_length;
    return data_end == buffer_end;
}

template<typename EventDetectionStrategyType, typename DataPointType> bool
EventDetector<EventDetectionStrategyType, DataPointType>::detectEvent(DataPointType *prev_period,
                                                                      DataPointType *current_period) {
    if (this->event_detection_strategy.detectEvent(current_period,
                                                   current_period + this->buffer_length,
                                                   this->buffer_length)) {
        std::cout << "time: " << this->dynamic_meta_data->getDataPointTime(this->data_points_read) << std::endl;

        return true;

    }
    return false;
}

template<typename EventDetectionStrategyType, typename DataPointType> void
EventDetector<EventDetectionStrategyType, DataPointType>::storeEvent(DataPointType *prev_period,
                                                                     DataPointType *current_period) {
    // Make sure we want to store at least one period of data
    if (this->power_meta_data.data_points_stored_of_event <= 0 || this->stop_now) { return; }

    int total_data_points_stored = this->dynamic_meta_data->getFixedPowerMetaData().data_points_stored_before_event;
    total_data_points_stored += this->dynamic_meta_data->getFixedPowerMetaData().data_points_stored_of_event;
    std::unique_ptr<DataPointType[]> data_points(new DataPointType[total_data_points_stored]);


    this->data_manager->popDataPoints(data_points.get(), data_points.get() + total_data_points_stored);
    this->storage.storeEvent(data_points.get(), data_points.get() + total_data_points_stored,
                             EventMetaData(this->dynamic_meta_data->getDataPointTime(this->data_points_read)));
}

template<typename EventDetectionStrategyType, typename DataPointType> void
EventDetector<EventDetectionStrategyType, DataPointType>::stopGracefully() {
    this->continue_analyzing = false;
    this->runner.join();
}

template<typename EventDetectionStrategyType, typename DataPointType> void
EventDetector<EventDetectionStrategyType, DataPointType>::stopNow() {
    this->continue_analyzing = false;
    this->stop_now = true;

    this->data_manager->unblockReadOperations();
    this->runner.join();
}

#endif // EVENTDETECTOR_H
