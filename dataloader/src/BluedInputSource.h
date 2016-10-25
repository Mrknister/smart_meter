#ifndef _BLUED_INPUT_SOURCE_
#define _BLUED_INPUT_SOURCE_

#include <string>
#include <thread>
#include "DataManager.h"


class BluedInputSource {
    
public:
    void startReading(const std::string& file_path, DataManager& mgr);
private:
    void run(const std::string& file_path, DataManager& mgr);
    static DataPoint matchLine(std::ifstream& input_stream);
    static bool skipToData(std::ifstream& input_stream);
private:
    bool continue_reading = true;
    std::thread runner;
};

#endif // _BLUED_INPUT_SOURCE_
