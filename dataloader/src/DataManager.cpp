#include "DataManager.h"

#include <iostream>

void DataManager::setConfig(const PowerMeterData& config)
{
    std::lock_guard<std::mutex> l(this->data_queue_mutex);
    this->conf = config;
}
PowerMeterData DataManager::getConfig()
{
    return conf;
}

std::vector<DataPoint> DataManager::getElectricalPeriod()
{
    return this->getDataPoints(this->conf.data_points_per_period);
}

void DataManager::nextElectricalPeriod()
{
    this->nextDataPoints(this->conf.data_points_per_period);
}

std::vector<DataPoint> DataManager::getDataPoints(unsigned int num_data_points)
{

    std::unique_lock<std::mutex> deque_overflow_wait_lock(this->data_queue_mutex);

    auto queue_has_enough_elements = [this, num_data_points]()-> bool {
        return this->data_queue.size() >= num_data_points;
    };

    this->deque_underflow.wait(deque_overflow_wait_lock, queue_has_enough_elements);

    std::vector<DataPoint> result(data_queue.begin(), data_queue.begin() + num_data_points);
    return result;
}

void DataManager::nextDataPoints(unsigned int num_data_points)
{
    std::unique_lock<std::mutex> deque_overflow_wait_lock(this->data_queue_mutex);

    auto queue_has_enough_elements = [this, num_data_points]()-> bool {
        return this->data_queue.size() >= num_data_points;
    };

    this->deque_underflow.wait(deque_overflow_wait_lock, queue_has_enough_elements);
    this->data_queue.erase(data_queue.begin(), data_queue.begin() + num_data_points);
    this->deque_overflow.notify_one();
}


void DataManager::addDataPoint(DataPoint data_point)
{
    std::unique_lock<std::mutex> deque_overflow_wait_lock(this->data_queue_mutex);

    auto queue_not_full = [this]()-> bool {
        return this->data_queue.size() < this->conf.max_data_points_in_queue;
    };
    int size = this->data_queue.size();

    this->deque_overflow.wait(deque_overflow_wait_lock, queue_not_full);

    this->data_queue.push_back(data_point);
    this->deque_underflow.notify_one();

}

