#ifndef SMART_SCREEN_BLUEDHDF5INPUTSOURCE_H
#define SMART_SCREEN_BLUEDHDF5INPUTSOURCE_H

#include <functional>
#include <thread>
#include "DynamicStreamMetaData.h"
#include <H5Cpp.h>

#include "BluedDefinitions.h"

class BluedHdf5InputSource {
public:

    void startReading(const std::string &file_path);
    void startReading(const std::string &file_path, std::function<void()> callback);

    void stopNow();

    void stopGracefully();

    ~BluedHdf5InputSource() {
        this->stopNow();
        this->stopGracefully();
    }


public:
    BluedDataManager data_manager;
    DynamicStreamMetaData meta_data;


private:
    void run(const std::string &file_path, std::function<void()> callback);

    bool readOnce(H5::DataSet dataset, H5::DataSpace &dataspace);
    void updateDynamicStreamMetaData(BluedDataPoint to_update);
    void initStartValues();

private:
    bool continue_reading = true;
    std::thread runner;
    hsize_t data_set_size[2];
    hsize_t current_offset[2];
    static const unsigned int buffer_size = 1000;
    BluedDataPoint buffer[buffer_size];
    static const int fields = 4;
    DynamicStreamMetaData::TimeType start_time;

    hsize_t mem_space_dimensions[2] = {buffer_size, fields};

    H5::DataSpace memspace = H5::DataSpace(2, mem_space_dimensions);

};


#endif //SMART_SCREEN_BLUEDHDF5INPUTSOURCE_H
