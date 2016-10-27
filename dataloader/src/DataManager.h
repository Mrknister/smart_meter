#ifndef _DATAMANAGER_H_
#define _DATAMANAGER_H_

#include <vector>
#include <deque>
#include <thread>
#include <shared_mutex>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <boost/optional.hpp>


#include "DataPoint.h"
#include "PowerMeterData.h"

/**
 * @brief The DataManager is a synchronized point for asynchronous reading and writing operations.
 *
 */
template<typename DataPointType>
class  DataManager;

typedef  DataManager<DataPoint> DefaultDataManager;

template<typename DataPointType>
class  DataManager {
public:
    ~DataManager();
    /**
     * @brief Sets the config. This function blocks until all functions that need to read the config have completed.
     *
     * @param config The configuration that is to be set.
     *
     */
    void setConfig(const PowerMeterData& config);

    /**
     * @brief Copies the config to the user.
     *
     * @returns A copy of the current configuration.
     */
    PowerMeterData getConfig();

    /**
     * @brief blocks until a full period of electrical data can be read and returns it in a vector.
     */
    std::vector<DataPointType> getElectricalPeriod();

    /**
     * @brief Removes one period from the queue
     */
    void nextElectricalPeriod();


    /**
     * @brief blocks until the number of data points has been reached.
     */
    std::vector<DataPointType> getDataPoints(unsigned int num_data_points);


    template<class IteratorType>
    void getDataPoints(IteratorType begin, IteratorType end);

    /**
     * @brief Removes num_data_points data points from the queue
     */
    void nextDataPoints(unsigned int num_data_points);


    /**
     * @brief Adds a datapoint to the back of the queue. Blocks until less than PowerMeterData::max_data_points_in_queue is in the queue.
     *
     * @param data_point The datapoint that is to be added.
     *
     */
    void addDataPoint(DataPointType data_point);

    /**
     * @brief Adds multiple data points to the queue.
     *
     * @param data_point The datapoint that is to be added.
     *
     */
    template<typename IteratorType>
    void addDataPoints(IteratorType begin, IteratorType end);





private:
    std::function<bool()>  getQueueHasEnoughElementsWaiter(unsigned int num_elements);
    std::function<bool()>  getQueueNotFullWaiter();
    std::function<bool()>  getQueueNotEmptyWaiter();


private:
    std::condition_variable deque_overflow;
    std::condition_variable deque_underflow;
    std::mutex data_queue_mutex;
    std::deque<DataPointType> data_queue;
    bool stream_ended = true;

    PowerMeterData conf;

};


template<typename DataPointType>
template<typename IteratorType>
void DataManager<DataPointType>::addDataPoints(IteratorType begin, IteratorType end)
{
    auto waiting_function = this->getQueueNotFullWaiter();

    while(begin != end) {
        std::unique_lock<std::mutex> deque_overflow_wait_lock(this->data_queue_mutex);
        this->deque_overflow.wait(deque_overflow_wait_lock, waiting_function);
        long pushable_elements =  this->conf.max_data_points_in_queue - this->data_queue.size();
        long elements_pushed = std::min(end - begin, pushable_elements);
        this->data_queue.insert(data_queue.end(), begin , begin+ elements_pushed);
        begin += elements_pushed;
        this->deque_underflow.notify_one();
    }
}


template<typename DataPointType>
DataManager<DataPointType>::~DataManager()
{
    {
        std::lock_guard<std::mutex> clear_queue(data_queue_mutex);
        data_queue.clear();
    }
    deque_overflow.notify_all();
}

template<typename DataPointType>
void DataManager<DataPointType>::setConfig(const PowerMeterData& config)
{
    std::lock_guard<std::mutex> l(this->data_queue_mutex);
    this->conf = config;
}

template<typename DataPointType>
PowerMeterData DataManager<DataPointType>::getConfig()
{
    return conf;
}

template<typename DataPointType>
std::vector<DataPointType> DataManager<DataPointType>::getElectricalPeriod()
{
    return this->getDataPoints(this->conf.dataPointsPerPeriod());
}

template<typename DataPointType>
void DataManager<DataPointType>::nextElectricalPeriod()
{
    this->nextDataPoints(this->conf.dataPointsPerPeriod());
}

template<typename DataPointType>
std::vector<DataPointType> DataManager<DataPointType>::getDataPoints(unsigned int num_data_points)
{

    std::unique_lock<std::mutex> deque_overflow_wait_lock(this->data_queue_mutex);

    auto queue_has_enough_elements = this->getQueueHasEnoughElementsWaiter(num_data_points);
    this->deque_underflow.wait(deque_overflow_wait_lock, queue_has_enough_elements);

    std::vector<DataPoint> result(data_queue.begin(), data_queue.begin() + num_data_points);
    return result;
}

template<typename DataPointType>
template<typename IteratorType>
void DataManager<DataPointType>::getDataPoints(IteratorType begin, IteratorType end)
{
    auto waiting_function = this->getQueueNotEmptyWaiter();

    while(begin != end) {
        std::unique_lock<std::mutex> deque_overflow_wait_lock(this->data_queue_mutex);
        this->deque_underflow.wait(deque_overflow_wait_lock, waiting_function);
        long pullable_elements =  this->conf.max_data_points_in_queue - this->data_queue.size();
        long elements_pulled = std::min(end - begin, pullable_elements);
        this->data_queue.insert(data_queue.end(), begin , begin+ elements_pulled);
        begin += elements_pulled;
    }
}

template<typename DataPointType>
void DataManager<DataPointType>::nextDataPoints(unsigned int num_data_points)
{
    std::unique_lock<std::mutex> deque_overflow_wait_lock(this->data_queue_mutex);

    auto queue_has_enough_elements = this->getQueueHasEnoughElementsWaiter(num_data_points);
    this->deque_underflow.wait(deque_overflow_wait_lock, queue_has_enough_elements);
    this->data_queue.erase(data_queue.begin(), data_queue.begin() + num_data_points);

    this->deque_overflow.notify_one();
}


template<typename DataPointType>
void DataManager<DataPointType>::addDataPoint(DataPointType data_point)
{
    std::unique_lock<std::mutex> deque_overflow_wait_lock(this->data_queue_mutex);

    auto queue_not_full = this->getQueueNotFullWaiter();
    this->deque_overflow.wait(deque_overflow_wait_lock, queue_not_full);

    this->data_queue.push_back(data_point);
    this->deque_underflow.notify_one();

}

template<typename DataPointType>
std::function<bool ()> DataManager<DataPointType>::getQueueHasEnoughElementsWaiter(unsigned int num_elements)
{
    return [this, num_elements]()-> bool {
        return this->data_queue.size() >= num_elements;
    };
}

template<typename DataPointType>
std::function<bool ()> DataManager<DataPointType>::getQueueNotEmptyWaiter()
{
    return [this]()-> bool {
        return this->data_queue.size() > 0;
    };
}

template<typename DataPointType>
std::function<bool ()> DataManager<DataPointType>::getQueueNotFullWaiter()
{
    return [this]()-> bool {
        return this->data_queue.size() < this->conf.max_data_points_in_queue;
    };
}


#endif // _DATAMANAGER_H_
