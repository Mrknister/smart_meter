#include <iostream>
#include "PowerMeterData.h"
#include "DataManager.h"

int main(int argc, char** argv)
{
    using namespace std;
    
    if(argc <2) {
        std::cout << "Please pass the location of the config file.\n";
        return 0;
    }
    
    
    PowerMeterData conf;
    conf.load(argv[1]);
    
    std::cout << "Amps:" << conf.scale_amps <<endl;
    
    return 0;
}
