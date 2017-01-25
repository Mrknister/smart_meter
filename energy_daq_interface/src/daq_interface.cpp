
#define DEBUG_OUTPUT
#include <AsyncDataQueue.h>
#include <DataClassifier.h>
#include <EventDetector.h>
#include "MEDALDataPoint.h"
#include "daq_interface.h"
#include <iostream>
#include <iomanip>

AsyncDataQueue<MEDALDataPoint> data_queue;
DynamicStreamMetaData stream_meta_data;
EventDetector<DefaultEventDetectionStrategy,MEDALDataPoint> event_detector;
DataClassifier<MEDALDataPoint> event_analyzer;
#define MEDAL_BUFFER_SIZE 5000
int buffer_pos = 0;
MEDALDataPoint buffer[MEDAL_BUFFER_SIZE];
DynamicStreamMetaData::DataPointIdType data_point_id = 0;



extern "C" void init_daq_interface(unsigned int sample_rate) {
    data_queue.setQueueMaxSize(sample_rate*3);
    PowerMetaData meta_data;
    meta_data.sample_rate = sample_rate;
    meta_data.frequency = 50;
    meta_data.data_points_stored_before_event = sample_rate/2;
    meta_data.data_points_stored_of_event = sample_rate;
    meta_data.scale_amps = 1;
    meta_data.scale_volts = 1;

    stream_meta_data.setFixedPowerMetaData(meta_data);
    stream_meta_data.syncTimePoint(0,boost::posix_time::second_clock::local_time());
    event_detector.storage.setEventStorageCallback([](Event<MEDALDataPoint>& event) {
        event_analyzer.pushEvent(event);
    });
    event_detector.startAnalyzing(&data_queue,&stream_meta_data,DefaultEventDetectionStrategy(0.3));

    event_analyzer.startClassification();

}

extern "C" void  addMEDALDataPoint(float current0, float current1, float current2, float current3, float current4, float current5,
                       float voltage) {
    MEDALDataPoint dp;
    dp.currents[0] = current0;
    dp.currents[1] = current1;
    dp.currents[2] = current2;
    dp.currents[3] = current3;
    dp.currents[4] = current4;
    dp.currents[5] = current5;
    dp.volts = voltage;

    buffer[buffer_pos] = dp;
    ++buffer_pos;
    ++data_point_id;
    if(buffer_pos == MEDAL_BUFFER_SIZE) {
        stream_meta_data.syncTimePoint(data_point_id, boost::posix_time::second_clock::local_time());
        data_queue.addDataPoints(buffer, buffer+MEDAL_BUFFER_SIZE);
        buffer_pos = 0;
    }
}

extern "C" void  free_daq_interface() {
    data_queue.notifyStreamEnd();
    event_detector.join();

}
