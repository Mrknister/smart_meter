#include <iostream>
#include <thread>

#include "PowerMeterData.h"
#include "DataManager.h"
#include "BluedHdf5InputSource.h"

#include "BluedToDefaultDataManagerAdapter.h"

int main(int argc, char** argv)
{
    using namespace std;
    
    if(argc <2) {
        std::cout << "Please pass the location of the config file.\n";
        return 0;
    }
    
    PowerMeterData conf;
    conf.load(argv[1]);
    BluedHdf5InputSource data_source;
    DefaultDataManager default_data_manager;
    data_source.startReading("all_001.hdf5");
    std::thread adapter_thread(&adaptBluedToDefaultDataManager, &data_source.data_manager, &default_data_manager);
    DataPoint buffer[1000];
    default_data_manager.popDataPoints(buffer, buffer + 1000);
    for (auto p: buffer) {
        std::cout << "Data Point:" << p.ampere << " " << p.volts<<endl;

    }
    adapter_thread.join();
    

    return 0;
}
