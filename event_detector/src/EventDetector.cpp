#include "EventDetector.h"
#include "Algorithms.h"

void EventDetector::startAnalyzing(DefaultDataManager *data_manager, PowerMetaData meta_data,
                                   EventMetaData::TimeType start_time) {
    this->data_manager = data_manager;
    this->power_meta_data = meta_data;
    this->electrical_period_buffer1 = std::unique_ptr<DefaultDataPoint>(
            new DefaultDataPoint[meta_data.dataPointsPerPeriod()]);
    this->electrical_period_buffer2 = std::unique_ptr<DefaultDataPoint>(
            new DefaultDataPoint[meta_data.dataPointsPerPeriod()]);
    this->start_time = start_time;
    runner = std::thread(&EventDetector::run, this);
}

void EventDetector::run() {
    DefaultDataPoint *buffer_prev_period = this->electrical_period_buffer1.get();
    DefaultDataPoint *buffer_current_period = this->electrical_period_buffer1.get();
    this->readBuffer(buffer_prev_period);
    while (this->readBuffer(buffer_current_period)) {
        if (this->detectEvent(buffer_current_period)) {
            this->storeEvent(buffer_prev_period, buffer_current_period);
        }
    }
}

bool EventDetector::readBuffer(DefaultDataPoint *data_point) {
    DefaultDataPoint *buffer_end = data_point + this->power_meta_data.dataPointsPerPeriod();
    DefaultDataPoint *data_end = data_manager->popDataPoints(data_point, buffer_end);
    this->periods_read += 1;
    return data_end == buffer_end;
}

bool EventDetector::detectEvent(DefaultDataPoint *period) {
    float rms = Algorithms::rootMeanSquare(period, period + this->power_meta_data.dataPointsPerPeriod());
    static float prev_rms = 0;
    if (rms - prev_rms > 2) {
        std::cout << "time: " << this->getCurrentTime() << " = " <<this->start_time << " + " << this->getTimePassed().total_milliseconds() / 1000.f<<"\nprev_rms: " << prev_rms << " current_rms: " << rms
                  << std::endl;
        prev_rms = rms;

        return true;

    }
    prev_rms = rms;
    return false;
}

void EventDetector::storeEvent(DefaultDataPoint *prev_period, DefaultDataPoint *current_period) {
    if (this->power_meta_data.periods_stored <= 0) { return; }
    // store the two periods we already accuired in a buffer_vector
    std::vector<DefaultDataPoint> to_store(prev_period, prev_period + this->power_meta_data.dataPointsPerPeriod());
    to_store.insert(to_store.end(), current_period, current_period + this->power_meta_data.dataPointsPerPeriod());
    to_store.resize((this->power_meta_data.periods_stored + 1) * this->power_meta_data.dataPointsPerPeriod());
    //get the rest of the periods we want

    this->data_manager->popDataPoints(to_store.begin()+this->power_meta_data.dataPointsPerPeriod()*2, to_store.end());
    this->storage.storeEvent(to_store.begin(), to_store.end(),EventMetaData(this->getCurrentTime()));

}

boost::posix_time::time_duration EventDetector::getTimePassed() {
    return boost::posix_time::seconds(periods_read) / this->power_meta_data.frequency;
}

EventMetaData::TimeType EventDetector::getCurrentTime() {
    return this->start_time + this->getTimePassed();
}

