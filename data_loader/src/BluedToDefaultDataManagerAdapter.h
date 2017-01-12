#ifndef SMART_SCREEN_BLUEDTODEFAULTDATAMANAGERADAPTER_H
#define SMART_SCREEN_BLUEDTODEFAULTDATAMANAGERADAPTER_H

#include <algorithm>


#include "AsyncDataQueue.h"
#include "BluedDefinitions.h"
#include "DefaultDataPoint.h"

void adaptBluedToDefaultDataManager(BluedDataManager* blued_data_mgr, DefaultDataManager* default_data_mgr);

void adaptBluedToDefaultDataManager(BluedDataManager* blued_data_mgr, DefaultDataManager* default_data_mgr) {
    const int buffer_size = 1000;
    BluedDataPoint blued_buffer[buffer_size];
    DefaultDataPoint data_point_buffer[buffer_size];
    BluedDataPoint *blued_buffer_end = blued_buffer + buffer_size;
    BluedDataPoint *blued_buffer_read_end = blued_buffer + buffer_size;

    while (blued_buffer_end == blued_buffer_read_end) {
        blued_buffer_read_end = blued_data_mgr->popDataPoints(blued_buffer, blued_buffer_end);

        std::transform(blued_buffer, blued_buffer_end, data_point_buffer, [](BluedDataPoint data_point){
            return DefaultDataPoint(data_point.voltage_a, data_point.current_a);
        });

        default_data_mgr->addDataPoints(data_point_buffer, data_point_buffer + buffer_size);
    }

    default_data_mgr->notifyStreamEnd();
}

#endif //SMART_SCREEN_BLUEDTODEFAULTDATAMANAGERADAPTER_H
