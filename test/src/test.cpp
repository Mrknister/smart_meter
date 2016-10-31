#include "test.h"


#define BOOST_TEST_MODULE Data Loader Test

#include <boost/test/included/unit_test.hpp>

#include <ctime>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <boost/range/counting_range.hpp>

#define private public

#include "DataManager.h"
#include "PowerMeterData.h"
#include "BluedInputSource.h"
#include "BluedHdf5Converter.h"
#include "BluedHdf5InputSource.h"


void fucker() {
    using namespace std;
    using namespace boost::range;
    DataManager<int> data_mgr;
    const int range_size = 12049;
    const int cycles = 60;

    clock_t start = clock();

    std::thread runner([&data_mgr]() {
        for (int i = 0; i < cycles; ++i) {
            auto range = boost::counting_range(i * range_size, i * range_size + range_size);
            data_mgr.addDataPoints(range.begin(), range.end());
        }
        data_mgr.notifyStreamEnd();
    });

    int elements_read = 0;
    int total_elements_read = 0;
    const int want_to_read = range_size - 11;
    data_mgr.setQueueMaxSize(want_to_read);


    std::vector<int> counter;

    do {
        counter.resize(counter.size() + want_to_read);
        std::vector<int>::iterator write_pos = counter.begin() + total_elements_read;


        auto elements_end_iter = data_mgr.popDataPoints(write_pos, counter.end());
        elements_read = elements_end_iter - write_pos;
        total_elements_read += elements_read;

    } while (elements_read);

    counter.resize(total_elements_read);


    double time = (double) (clock() - start) / ((double) CLOCKS_PER_SEC);
    BOOST_TEST_MESSAGE("Time taken: " << time);

    BOOST_TEST(vec_is_sorted(counter));

    BOOST_TEST(counter.size() == range_size * cycles);
    if (runner.joinable())
        runner.join();
}

BOOST_AUTO_TEST_CASE(test_read_write_sorted) {
    fucker();
}

BOOST_AUTO_TEST_CASE(test_blued_data_source) {
    BluedInputSource source;
    source.startReading("location_001_dataset_001/location_001_ivdata_001.txt/data", [](){});
    const int buffer_size = 240;
    BluedDataPoint buffer [buffer_size];
    const int num_data_points_tested = 12000*60;
    clock_t start = clock();

    for(int i= 0; i<num_data_points_tested /buffer_size; ++i) {
        source.data_manager.popDataPoints(buffer, buffer + buffer_size);
    }
    double time = (double) (clock() - start) / ((double) CLOCKS_PER_SEC);
    BOOST_TEST_MESSAGE("Time taken with csv: " << time);

}


BOOST_AUTO_TEST_CASE(test_convert_file) {
    BluedHdf5Converter converter;
    converter.convertToHdf5("test_convert", "out.hdf5");
}

/* // Takes very long
BOOST_AUTO_TEST_CASE(test_convert_mutch) {
    BluedHdf5Converter converter;
    converter.convertToHdf5("/home/jan/Downloads/location_001_dataset_001", "all_001.hdf5");
}*/

BOOST_AUTO_TEST_CASE(test_hdf5_input_source_test) {
    BluedHdf5InputSource source;
    source.startReading("all_001.hdf5", []() {});
    const int buffer_size = 240;
    BluedDataPoint buffer [buffer_size];
    const int num_data_points_tested = 12000*60*60;
    clock_t start = clock();

    for(int i= 0; i<num_data_points_tested / buffer_size; ++i) {
        source.data_manager.popDataPoints(buffer, buffer+buffer_size);
    }
    double time = (double) (clock() - start) / ((double) CLOCKS_PER_SEC);
    BOOST_TEST_MESSAGE("Time taken with hdf5: " << time);
}
