#include <DataManager.h>
#include <DataAnalyzer.h>
#include <EventDetector.h>
#include "MEDALDataPoint.h"
#include "daq_interface.h"
#include <iostream>
#include <iomanip>

DataManager<MEDALDataPoint> data_queue;
DynamicStreamMetaData stream_meta_data;
EventDetector<DefaultEventDetectionStrategy,MEDALDataPoint> event_detector;

extern "C" void init_daq_interface() {
    data_queue.setQueueMaxSize(100000);
    PowerMetaData meta_data;
    meta_data.sample_rate = 50000;
    meta_data.frequency = 50;
    meta_data.data_points_stored_before_event = 25000;
    meta_data.data_points_stored_of_event = 50000;
    meta_data.scale_amps = 1;
    meta_data.scale_volts = 1;
    stream_meta_data.setFixedPowerMetaData(meta_data);
    stream_meta_data.syncTimePoint(0,boost::posix_time::second_clock::local_time());
    event_detector.startAnalyzing(&data_queue,&stream_meta_data);



}

void addMEDALDataPoint(float current0, float current1, float current2, float current3, float current4, float current5,
                       float voltage) {
    MEDALDataPoint dp;
    dp.currents[0] = current0;
    dp.currents[1] = current1;
    dp.currents[2] = current2;
    dp.currents[3] = current3;
    dp.currents[4] = current4;
    dp.currents[5] = current5;
    dp.volts = voltage;
    data_queue.addDataPoint(dp);
    for(int i = 0; i <= 5; ++i) {

        std::cout << std::setfill(' ') << std::left << std::setw(7)<<dp.currents[i]  << "  ";
    }
    std::cout << std::setfill(' ') << std::setw(7) <<voltage<< std::endl;


}

void free_daq_interface() {
    data_queue.notifyStreamEnd();
    event_detector.join();

}
