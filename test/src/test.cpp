
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


BOOST_AUTO_TEST_CASE(test_convert_file) {
    BluedHdf5Converter converter;
    converter.convertToHdf5("location_001_dataset_001/location_001_ivdata_001.txt", "out.hdf5");
}


