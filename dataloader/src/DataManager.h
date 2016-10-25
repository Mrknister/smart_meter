
#ifndef _DATAMANAGER_H_
#define _DATAMANAGER_H_

#include <vector>
#include <deque>
#include <thread>
#include <shared_mutex>
#include <mutex>
#include <condition_variable>


#include "DataPoint.h"
#include "PowerMeterData.h"

/**
 * @brief The DataManager is a synchronized point for asynchronous reading and writing operations.
 * 
 */
class  DataManager {
public:
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
    std::vector<DataPoint> getElectricalPeriod();
    
    /**
     * @brief Removes one period from the queue
     */
    void nextElectricalPeriod();
    
    
    /**
     * @brief blocks until the number of data points has been reached.
     */
    std::vector<DataPoint> getDataPoints(unsigned int num_data_points);
    
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
    void addDataPoint(DataPoint data_point);

    /**
     * @brief Adds multiple data points to the queue.
     * 
     * @param data_point The datapoint that is to be added.
     * 
     */
    template <class IteratorType>
    void addDataPoints(IteratorType begin, IteratorType end);
    

private:
    std::condition_variable deque_overflow;
    std::condition_variable deque_underflow;
    std::deque<DataPoint> data_queue;
    std::mutex data_queue_mutex;

    PowerMeterData conf;

};

template<class IteratorType> void DataManager::addDataPoints(IteratorType begin, IteratorType end)
{
    while(begin != end) {
        addDataPoint(*begin);
        ++begin;
    }
}



#endif // _DATAMANAGER_H_
