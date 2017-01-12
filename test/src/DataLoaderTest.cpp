#include "DataLoaderTest.h"


#include <boost/test/included/unit_test.hpp>

#include <ctime>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <boost/range/counting_range.hpp>
#include <cstring>
#include <boost/filesystem.hpp>


#define private public

#include "AsyncDataQueue.h"
#include "PowerMetaData.h"
#include "BluedInputSource.h"
#include "../../blued_converter/src/BluedHdf5Converter.h"
#include "BluedHdf5InputSource.h"

BOOST_AUTO_TEST_SUITE(data_loader_suite)

    void createDataSet(std::string base_dir,const int data_set_num, const int num_data_points_per_set);


    BOOST_AUTO_TEST_CASE(test_read_write_sorted) {
        using namespace std;
        using namespace boost::range;
        AsyncDataQueue<int> data_mgr;
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


        double time = static_cast<double>(clock() - start) / static_cast<double>(CLOCKS_PER_SEC);
        BOOST_TEST_MESSAGE("Time taken: " << time);

        BOOST_TEST(vec_is_sorted(counter));

        BOOST_TEST(counter.size() == range_size * cycles);
        if (runner.joinable()) {
            runner.join();
        }
    }


/*
    BOOST_AUTO_TEST_CASE(test_convert_file) {
        BluedHdf5Converter converter;
        converter.convertToHdf5("test_convert", "out.hdf5");
    }*/
/*
    BOOST_AUTO_TEST_CASE(test_convert_mutch) {
        BluedHdf5Converter converter;
        converter.convertToHdf5("/home/jan/Downloads/location_001_dataset_001", "all_001.hdf5");
    }

    BOOST_AUTO_TEST_CASE(test_hdf5_input_source_test) {
        BluedHdf5InputSource source;
        source.startReading("all_001.hdf5", []() {});
        const int buffer_size = 240;
        BluedDataPoint buffer[buffer_size];
        const int num_data_points_tested = 12000 * 60 * 60;
        clock_t start = clock();

        for (int i = 0; i < num_data_points_tested / buffer_size; ++i) {
            source.data_manager.popDataPoints(buffer, buffer + buffer_size);
        }
        double time = static_cast<double>(clock() - start) / static_cast<double>(CLOCKS_PER_SEC);
        BOOST_TEST_MESSAGE("Time taken with hdf5: " << time);
    }*/

    BOOST_AUTO_TEST_CASE(test_hdf5_csv_equality) {
        BluedHdf5InputSource source;
        PowerMetaData conf;
        BOOST_TEST(conf.load("config.ini"));
        source.meta_data.setFixedPowerMetaData(conf);

        source.startReading("all_001.hdf5", []() {});

        BluedInputSource csv_source;
        csv_source.readWholeLocation("/home/jan/Documents/Bachelor Thesis/location_001_dataset_001");
        const int buffer_size = 240;
        BluedDataPoint buffer1[buffer_size];
        BluedDataPoint buffer2[buffer_size];

        const int num_data_points_tested = 12000 * 60 * 3;

        for (int i = 0; i < num_data_points_tested / buffer_size; ++i) {
            source.data_manager.popDataPoints(buffer1, buffer1 + buffer_size);
            csv_source.data_manager.popDataPoints(buffer2, buffer2 + buffer_size);
            bool buffers_equal = memcmp(buffer1, buffer2, sizeof(BluedDataPoint) * buffer_size) == 0;
            BOOST_TEST(buffers_equal);
        }
        BOOST_TEST_MESSAGE("total data points compared: " << (num_data_points_tested / buffer_size - 1) * buffer_size);
    }

    void createDataSet(std::string base_dir, const int data_set_num, const int num_data_points_per_set) {
        const int base_data_point_number = data_set_num * num_data_points_per_set;
        std::string dir_name = base_dir + std::to_string(data_set_num) + ".txt";
        boost::filesystem::create_directories(dir_name);

        std::ofstream data_set(dir_name + "/data");
        data_set << "X_Value,Current A,Current B,VoltageA,Comment\r\n";
        for (int i = 0; i < num_data_points_per_set; ++i) {
            data_set << static_cast<float>(base_data_point_number + i) + 0.1 << ","
                     << static_cast<float>(base_data_point_number + i) + 0.1 << ","
                     << static_cast<float>( base_data_point_number + i) + 0.1 << ","
                     << static_cast<float>( base_data_point_number + i) + 0.1 << "\r\n";
        }
        data_set.close();
    }

    BOOST_AUTO_TEST_CASE(test_read_whole_location) {
        const int num_data_sets = 9;
        const int data_points_per_set = 100;
        const int buffer_size = num_data_sets * data_points_per_set + 10;
        BluedDataPoint buffer[buffer_size];
        std::string test_dir("location_test/");
        for (int i = 0; i < num_data_sets; ++i) {
            createDataSet(test_dir, i, data_points_per_set);
        }
        BluedInputSource source;
        source.readWholeLocation(test_dir);

        BluedDataPoint *buffer_end = source.data_manager.popDataPoints(buffer, buffer + buffer_size);
        BOOST_TEST(buffer_end == buffer + num_data_sets * data_points_per_set);
        for (int i = 0; i < num_data_sets * data_points_per_set; ++i) {
            BOOST_TEST(static_cast<int>(buffer[i].voltage_a) == i);
        }
        boost::filesystem::remove_all(test_dir);
    }

BOOST_AUTO_TEST_SUITE_END()