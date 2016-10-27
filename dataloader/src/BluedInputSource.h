#ifndef _BLUED_INPUT_SOURCE_
#define _BLUED_INPUT_SOURCE_

#include <string>
#include <thread>
#include "DataManager.h"


class BluedInputSource {

public:
    void startReading(const std::string& file_path, DefaultDataManager& mgr);
    void startReading(const std::string& file_path, DefaultDataManager& mgr, std::function<void ()> callback);
    void readWholeLocation(const std::string& directory, DefaultDataManager& mgr, std::function<void ()> callback);
    void readWholeLocation(const std::string& directory, DefaultDataManager& mgr);

    void stopGracefully();
    void stopReading();


private:
    void run(const std::string& file_path, DefaultDataManager& mgr, std::function<void ()> callback);
    void runLocations(std::vector<std::string> locations, DefaultDataManager& mgr, std::function<void ()> callback);

    static DataPoint matchLine(std::ifstream& input_stream);

    bool readOnce(std::ifstream& input_stream, DefaultDataManager & mgr);

    static bool skipToData(std::ifstream& input_stream);
private:
    bool continue_reading = true;
    std::thread runner;
};

#endif // _BLUED_INPUT_SOURCE_
