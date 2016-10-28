#ifndef _BLUED_INPUT_SOURCE_
#define _BLUED_INPUT_SOURCE_

#include <string>
#include <thread>
#include <vector>

#include "DataManager.h"
#include "BluedDataPoint.h"

typedef DataManager<BluedDataPoint> BluedDataManager;

class BluedInputSource {


public:
    void startReading(const std::string& file_path, BluedDataManager& mgr);
    void startReading(const std::string& file_path, BluedDataManager& mgr, std::function<void ()> callback);
    void readWholeLocation(const std::string& directory, BluedDataManager& mgr, std::function<void ()> callback);
    void readWholeLocation(const std::string& directory, BluedDataManager& mgr);

    void stopGracefully();
    void stopReading();


private:
    void run(const std::string& file_path, BluedDataManager& mgr, std::function<void ()> callback);
    void readFile(const std::string& file_path, BluedDataManager& mgr);
    void runLocations(std::vector<std::string> locations, BluedDataManager& mgr, std::function<void ()> callback);

    static BluedDataPoint matchLine(std::ifstream& input_stream);

    bool readOnce(std::ifstream& input_stream, BluedDataManager & mgr);

    static bool skipToData(std::ifstream& input_stream);
private:
    bool continue_reading = true;
    std::thread runner;
};

#endif // _BLUED_INPUT_SOURCE_
