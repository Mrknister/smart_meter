#include "BluedHdf5InputSource.h"

#include <exception>

void BluedHdf5InputSource::startReading(const std::string &file_path) {
    this->startReading(file_path, []() {});
}

void BluedHdf5InputSource::startReading(const std::string &file_path, std::function<void()> callback) {
    this->continue_reading = true;
    auto runner_function = std::bind(&BluedHdf5InputSource::run, this, file_path, callback);
    this->runner = std::thread(runner_function);
}

void BluedHdf5InputSource::run(const std::string &file_path, std::function<void()> callback) {
    using namespace H5;
    H5File file(file_path, H5F_ACC_RDONLY);
    DataSet dataset = file.openDataSet("data");
    DataSpace dataspace = dataset.getSpace();
    this->current_offset[0] = 0;
    this->current_offset[1] = 0;

    int rank = dataspace.getSimpleExtentNdims();
    if (rank != 2) {
        throw std::exception();
    }

    dataspace.getSimpleExtentDims(this->data_set_size, NULL);
    if (this->data_set_size[1] != this->fields) {
        throw std::exception();
    }
    while (this->continue_reading) {this->readOnce(dataset, dataspace);}
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

    dataset.read(this->buffer, H5::PredType::NATIVE_FLOAT, memspace, dataspace);
    this->data_manager.addDataPoints(buffer, buffer + count[0]);

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

