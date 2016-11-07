#include <iostream>
#include <thread>

#include "PowerMetaData.h"
#include "DataManager.h"
#include "BluedHdf5InputSource.h"

#include "BluedToDefaultDataManagerAdapter.h"
#include "EventDetector.h"

int main(int argc, char** argv)
{

    using namespace std;

    if(argc <2) {
        std::cout << "Please pass the location of the config file.\n";
        return 0;
    }
    
    PowerMetaData conf;
    if(!conf.load(argv[1]))
    {
        std::cout << "Could not load config file: "<<argv[1]<<"\n";
        return -1;
    }

    std::cout <<  conf << endl;

    BluedHdf5InputSource data_source;
    DefaultDataManager default_data_manager;
    data_source.startReading("all_001.hdf5");
    std::thread adapter_thread(&adaptBluedToDefaultDataManager, &data_source.data_manager, &default_data_manager);

    EventDetector<DefaultEventDetectionStrategy> detect;
    detect.startAnalyzing(&default_data_manager,conf,DefaultEventDetectionStrategy(), boost::posix_time::time_from_string(conf.data_set_start_time));

    adapter_thread.join();
    

    return 0;
}
