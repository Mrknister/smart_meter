#include <iostream>
#include <thread>

#include "PowerMetaData.h"
#include "DataManager.h"
#include "BluedHdf5InputSource.h"

#include "BluedToDefaultDataManagerAdapter.h"
#include "EventDetector.h"
#include "FastFourierTransformCalculator.h"
#include <mcheck.h>

int main(int argc, char **argv) {

    using namespace std;


    if (argc < 2) {
        std::cout << "Please pass the location of the config file.\n";
        return 0;
    }

    PowerMetaData conf;
    if (!conf.load(argv[1])) {
        std::cout << "Could not load config file: " << argv[1] << "\n";
        return -1;
    }

    std::cout << conf << endl;

    BluedHdf5InputSource data_source;
    data_source.meta_data.setFixedPowerMetaData(conf);

    DefaultDataManager default_data_manager;
    default_data_manager.setQueueMaxSize(conf.max_data_points_in_queue);
    data_source.startReading("all_001.hdf5");
    std::thread adapter_thread(&adaptBluedToDefaultDataManager, &data_source.data_manager, &default_data_manager);

    EventDetector<> detect;
    detect.startAnalyzing(&default_data_manager, &data_source.meta_data);
    adapter_thread.join();


    return 0;
}
