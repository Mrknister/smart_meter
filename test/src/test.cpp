
#include "test.h"


#define BOOST_TEST_MODULE Data Loader Test
#include <boost/test/included/unit_test.hpp>

#include <ctime>
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>


#define private public
#include "DataManager.h"
#include "PowerMeterData.h"
#include "BluedInputSource.h"
#include "BluedHdf5Converter.h"



BOOST_AUTO_TEST_CASE(first_test)
{
    int i = 2;
    BOOST_TEST(i);
    BOOST_TEST(i == 2);

}

BOOST_AUTO_TEST_CASE(test_read_write_sorted)
{
    using namespace std;
    PowerMeterData meta_data;
    meta_data.sample_rate = 12000;
    meta_data.max_data_points_in_queue = 24000;
    DefaultDataManager data_mgr;
    data_mgr.setConfig(meta_data);
    const int num_data_points_tested = meta_data.sample_rate * 60;

    std::thread runner([&data_mgr, num_data_points_tested]() {
        for (int i = 0; i < num_data_points_tested; ++i) {
            data_mgr.addDataPoint(DataPoint((float) i, (float) i));
        }
    });
    std::vector<DataPoint> vec;

    clock_t start = clock();
    for (unsigned int i = 0; i < num_data_points_tested / meta_data.dataPointsPerPeriod();  ++i) {
        auto tmp = data_mgr.getElectricalPeriod();
        data_mgr.nextElectricalPeriod();
        BOOST_TEST(tmp.size() == meta_data.dataPointsPerPeriod());
        vec.insert(vec.end(), tmp.begin(), tmp.end());
    }
    double time = (double)(clock() - start) / ((double) CLOCKS_PER_SEC);
    BOOST_TEST_MESSAGE("Time taken: " << time);

    BOOST_TEST(vec.size() == num_data_points_tested);
    BOOST_TEST(vec_is_sorted(vec));
    if (runner.joinable())
        runner.join();
}


BOOST_AUTO_TEST_CASE(test_blued_input_source)
{

    using namespace std;
    PowerMeterData meta_data;
    meta_data.sample_rate = 12000;
    meta_data.max_data_points_in_queue = 12000;
    DefaultDataManager data_mgr;
    data_mgr.setConfig(meta_data);

    BluedInputSource input_source;
    input_source.startReading("data", data_mgr);


    const int num_data_points_tested = 240 * 500;

    std::vector<DataPoint> vec;
    for (unsigned int i = 0; i < num_data_points_tested / meta_data.dataPointsPerPeriod();  ++i) {
        auto tmp = data_mgr.getElectricalPeriod();
        data_mgr.nextElectricalPeriod();
        BOOST_TEST(tmp.size() == meta_data.dataPointsPerPeriod());
    }

    input_source.stopGracefully();


}

BOOST_AUTO_TEST_CASE(test_multiple_files)
{
    using namespace std;
    PowerMeterData meta_data;
    meta_data.sample_rate = 12000;
    meta_data.max_data_points_in_queue = 24000;
    DefaultDataManager data_mgr;
    data_mgr.setConfig(meta_data);


    BluedInputSource input_source;
    input_source.readWholeLocation("location_001_dataset_001", data_mgr);


    const int num_data_points_tested = meta_data.sample_rate * 60 * 5;

    clock_t start = clock();

    std::vector<DataPoint> vec;
    for (unsigned int i = 0; i < num_data_points_tested / meta_data.dataPointsPerPeriod();  ++i) {
        auto tmp = data_mgr.getElectricalPeriod();
        data_mgr.nextElectricalPeriod();
        BOOST_TEST(tmp.size() == meta_data.dataPointsPerPeriod());
    }
    double time = (double)(clock() - start) / ((double) CLOCKS_PER_SEC);
    BOOST_TEST_MESSAGE("Time taken: " << time);
    input_source.stopGracefully();




}

BOOST_AUTO_TEST_CASE(test_convert_file) {
    BluedHdf5Converter converter;
    converter.convertToHdf5("location_001_dataset_001/location_001_ivdata_001.txt", "out.hdf5");
}


