#include "BluedHdf5InputSource.h"

#include <exception>

void BluedHdf5InputSource::startReading(const std::string &file_path) {
    this->startReading(file_path, []() {});
}

void BluedHdf5InputSource::startReading(const std::string &file_path, std::function<void()> callback) {
    this->continue_reading = true;
    auto runner_function = std::bind(&BluedHdf5InputSource::run, this, file_path, callback);
    this->initStartValues();
    this->runner = std::thread(runner_function);
}

void BluedHdf5InputSource::run(const std::string &file_path, std::function<void()> callback) {
    using namespace H5;
    H5File file(file_path, H5F_ACC_RDONLY);
    DataSet dataset = file.openDataSet("data");
    DataSpace dataspace = dataset.getSpace();

    int rank = dataspace.getSimpleExtentNdims();
    if (rank != 2) {
        throw std::exception();
    }

    dataspace.getSimpleExtentDims(this->data_set_size, NULL);
    if (this->data_set_size[1] != this->fields) {
        throw std::exception();
    }
    while (this->continue_reading) { this->readOnce(dataset, dataspace); }
    callback();

}


bool BluedHdf5InputSource::readOnce(H5::DataSet dataset, H5::DataSpace &dataspace) {
    if (!this->continue_reading) {
        return false;
    }
    hsize_t read_count = this->buffer_size;
    if (this->buffer_size > this->data_set_size[0] - this->current_offset[0]) {
        read_count = this->data_set_size[0] - this->current_offset[0];
        this->continue_reading = false;
    }

    hsize_t count[2] = {read_count, this->data_set_size[1]};
    dataspace.selectHyperslab(H5S_SELECT_SET, count, current_offset);
    hsize_t offset_out[2] = {0, 0};
    this->memspace.selectHyperslab(H5S_SELECT_SET, count, offset_out);

    dataset.read(this->buffer, H5::PredType::NATIVE_FLOAT, this->memspace, dataspace);
    writeBufferToDataSet(count[0]);
    current_offset[0] += count[0];
    return this->continue_reading;

}

void BluedHdf5InputSource::stopNow() {
    this->continue_reading = false;
    this->data_manager.discardRestOfStream();

}

void BluedHdf5InputSource::stopGracefully() {
    if (this->runner.joinable()) {
        this->continue_reading = false;
        this->runner.join();
    }
}

void BluedHdf5InputSource::updateDynamicStreamMetaData(BluedDataPoint to_update) {
    DynamicStreamMetaData::DataPointIdType dp_id(this->current_offset[0] - 1);
    DynamicStreamMetaData::USDurationType time_passed(static_cast<int64_t>(to_update.x_value * 1000 * 1000));
    this->meta_data.syncTimePoint(dp_id, this->start_time + time_passed);
}

void BluedHdf5InputSource::initStartValues() {
    this->current_offset[0] = 0;
    this->current_offset[1] = 0;
    this->start_time = boost::posix_time::time_from_string(this->meta_data.getFixedPowerMetaData().data_set_start_time);
}

void BluedHdf5InputSource::writeBufferToDataSet(unsigned int num_data_points) {
    BluedDataPoint tmp_buffer[buffer_size];
    std::transform(&this->buffer[0], &this->buffer[0] + num_data_points, tmp_buffer,[](float* data_points){
        return BluedDataPoint(data_points[0],data_points[1],data_points[2],data_points[3]);
    });
    this->data_manager.addDataPoints(tmp_buffer, tmp_buffer + num_data_points);
    if(num_data_points > 0) {
        updateDynamicStreamMetaData(tmp_buffer[num_data_points - 1]);
    }
    


}

