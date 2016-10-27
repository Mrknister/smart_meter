#ifndef _DATAMANAGER_H_
#define _DATAMANAGER_H_

#include <deque>
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
    
    void setQueueMaxSize(unsigned long max_size);
    unsigned long getQueueMaxSize();
    unsigned long getQueueSize();


    /**
     * @brief Fills the iterator with data points. If the stream writing to this DataManager notifies the datamanager that no more data will be following, the returning iterator will point to the position after the last element.
     *
     * @param begin Iterator to first writing destination.
     * @param end Iterator end. If the running iterator equals end, no more data will be written.
     *
     */
    template<class IteratorType>
    IteratorType getDataPoints(IteratorType begin, IteratorType end);


    /**
     * @brief Removes num_data_points DataPoints from the queue
     *
     * @param num_data_points Number of data points to be removed.
     *
     */
    void nextDataPoints(unsigned long num_data_points);

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


    /**
     * @brief Unblocks calls to getDataPoints and nextData by safely returning the data already read for the former and removing the maximum possible amount of data for the latter.
     *
     * @param data_point The datapoint that is to be added.
     *
     */
    void unblockAllReadOperations();

    void notifyStreamEnd();


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
    unsigned long queue_max_size;
};



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
unsigned long DataManager<DataPointType>::getQueueMaxSize()
{
    return this->queue_max_size;
}

template<typename DataPointType>
unsigned long DataManager<DataPointType>::getQueueSize()
{
    std::unique_lock<std::mutex> queue_lock(this->data_queue_mutex);

    return this->queue->size();
}

template<typename DataPointType>
void DataManager<DataPointType>::setQueueMaxSize(unsigned long max_size)
{
    std::unique_lock<std::mutex> queue_lock(this->data_queue_mutex);
    this->queue_max_size = max_size;
    this->deque_overflow.notify_one();
}



template<typename DataPointType>
template<typename IteratorType>
void DataManager<DataPointType>::addDataPoints(IteratorType begin, IteratorType end)
{
    auto waiting_function = this->getQueueNotFullWaiter();

    while(begin != end) {
        std::unique_lock<std::mutex> deque_overflow_wait_lock(this->data_queue_mutex);
        this->deque_overflow.wait(deque_overflow_wait_lock, waiting_function);
        long pushable_elements =  this->queue_max_size - this->data_queue.size();
        long elements_pushed = std::min(end - begin, pushable_elements);
        this->data_queue.insert(data_queue.end(), begin , begin + elements_pushed);
        begin += elements_pushed;
        this->deque_underflow.notify_one();
    }
}


template<typename DataPointType>
template<typename IteratorType>
IteratorType DataManager<DataPointType>::getDataPoints(IteratorType begin, IteratorType end)
{
    auto waiting_function = this->getQueueNotEmptyWaiter();

    while(begin != end && !this->stream_ended) {
        std::unique_lock<std::mutex> deque_overflow_wait_lock(this->data_queue_mutex);
        this->deque_underflow.wait(deque_overflow_wait_lock, waiting_function);
        
        long pullable_elements =  this->data_queue.size();
        long elements_pulled = std::min(end - begin, pullable_elements);
        this->data_queue.insert(data_queue.end(), begin , begin + elements_pulled);
        begin += elements_pulled;
    }
    return begin;
}

template<typename DataPointType>
void DataManager< DataPointType >::nextDataPoints(unsigned long num_data_points)
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
        return this->data_queue.size() < this->queue_max_size;
    };
}

template <typename DataPointType>
void DataManager<DataPointType>::notifyStreamEnd() {
    this->stream_ended = true;
    this->deque_underflow.notify_all();
}


#endif // _DATAMANAGER_H_