
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


BOOST_AUTO_TEST_CASE ( first_test )
{
    int i = 2;
    BOOST_TEST ( i );
    BOOST_TEST ( i == 2 );

}

BOOST_AUTO_TEST_CASE ( test_fill_queue )
{
    using namespace std;
    DataManager data_mgr;
    unsigned int max = data_mgr.conf.max_data_points_in_queue;

    std::thread runner ( [&data_mgr]() {
        while ( true ) {
            data_mgr.addDataPoint ( DataPoint ( 5.,7. ) );
        }
    } );
    using namespace std::chrono_literals;
    runner.detach();
    std::unique_lock<std::mutex> deque_underflow_wait_lock ( data_mgr.data_queue_mutex );


    auto queue_not_full = [max, &data_mgr] ()-> bool {
        return data_mgr.data_queue.size() >= max+1;
    };

    data_mgr.deque_underflow.wait_for ( deque_underflow_wait_lock,100ms,queue_not_full );
    BOOST_TEST ( data_mgr.data_queue.size() == max );
}


BOOST_AUTO_TEST_CASE ( test_read_write_sorted )
{
    using namespace std;
    PowerMeterData meta_data;
    meta_data.data_points_per_period=50;
    meta_data.max_data_points_in_queue = 75;
    DataManager data_mgr;
    data_mgr.setConfig ( meta_data );
    const int num_data_points_tested = 1000000;

    std::thread runner ( [&data_mgr, num_data_points_tested]() {
        for ( int i = 0; i < num_data_points_tested; ++i ) {
            data_mgr.addDataPoint ( DataPoint ( ( float ) i, ( float ) i ) );
        }
    } );
    runner.detach();
    std::vector<DataPoint> vec;
    for ( unsigned int i = 0; i < num_data_points_tested/meta_data.data_points_per_period;  ++i ) {
        auto tmp = data_mgr.getElectricalPeriod();
        data_mgr.nextElectricalPeriod();
        BOOST_TEST ( tmp.size() == meta_data.data_points_per_period );
        vec.insert ( vec.end(), tmp.begin(), tmp.end() );
    }
    BOOST_TEST ( vec.size() == num_data_points_tested );
    BOOST_TEST ( vec_is_sorted ( vec ) );

}


BOOST_AUTO_TEST_CASE ( test_blued_input_source )
{
    using namespace std;
    PowerMeterData meta_data;
    meta_data.data_points_per_period=50;
    meta_data.max_data_points_in_queue = 75;
    DataManager data_mgr;
    data_mgr.setConfig ( meta_data );
    BluedInputSource input_source;
    input_source.startReading ( "data", data_mgr );

    const int num_data_points_tested = 10000;


    std::vector<DataPoint> vec;
    for ( unsigned int i = 0; i < num_data_points_tested/meta_data.data_points_per_period;  ++i ) {
        auto tmp = data_mgr.getElectricalPeriod();
        data_mgr.nextElectricalPeriod();

        BOOST_TEST ( tmp.size() == meta_data.data_points_per_period );
    }

}




