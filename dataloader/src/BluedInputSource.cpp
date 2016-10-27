#include "BluedInputSource.h"

#include <fstream>
#include <regex>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <algorithm>

void BluedInputSource::startReading(const std::string &file_path, DefaultDataManager &mgr) {
    this->startReading(file_path, mgr, []() {});
}


void
BluedInputSource::startReading(const std::string &file_path, DefaultDataManager &mgr, std::function<void()> callback) {
    this->continue_reading = true;
    auto runner_function = std::bind(&BluedInputSource::run, this, file_path, std::ref(mgr), callback);
    this->runner = std::thread(runner_function);
}

void BluedInputSource::readWholeLocation(const std::string &directory, DefaultDataManager &mgr) {
    this->readWholeLocation(directory, mgr, []() {});
}

void BluedInputSource::stopGracefully() {
    if (this->runner.joinable()) {
        this->continue_reading = false;
        this->runner.join();
    }
}

void BluedInputSource::stopReading() {
    this->continue_reading = false;

}

void BluedInputSource::readWholeLocation(const std::string &directory, DefaultDataManager &mgr,
                                         std::function<void()> callback) {
    using namespace boost::filesystem;
    path p(directory);
    std::vector<std::string> locations;
    try {
        if (!is_directory(p)) {
            std::cerr << "Please Provide a directory";
        }
        for (directory_entry &x : recursive_directory_iterator(p)) {
            bool is_data_file = x.path().filename() == "data";
            is_data_file &= boost::algorithm::ends_with(x.path().parent_path().filename().c_str(), "txt");
            if (is_data_file)
                locations.push_back(x.path().string());
        }

    } catch (const filesystem_error &ex) {
        std::cerr << ex.what() << '\n';
    }

    std::sort(locations.begin(), locations.end());

    this->continue_reading = true;
    auto runner_function = std::bind(&BluedInputSource::runLocations, this, locations, std::ref(mgr), callback);
    this->runner = std::thread(runner_function);
}

void BluedInputSource::runLocations(std::vector<std::string> locations, DefaultDataManager &mgr,
                                    std::function<void()> callback) {
    for (auto &file_path : locations) {
        if (!this->continue_reading) {
            break;
        }
        run(file_path, mgr, []() {});
    }
    callback();
}


void BluedInputSource::run(const std::string &file_path, DefaultDataManager &mgr, std::function<void()> callback) {
    readFile(file_path, mgr);
    callback();
}

DataPoint BluedInputSource::matchLine(std::ifstream &input_stream) {
    std::string line;

    // read csv values
    // ignore time
    input_stream.ignore(50, ',');

    float current_a;
    input_stream >> current_a;
    input_stream.ignore(50, ',');

    //ignore current b 
    input_stream.ignore(50, ',');

    float voltage_a;
    input_stream >> voltage_a;

    // skip to next line
    input_stream.ignore(10, '\r');

    return DataPoint(voltage_a, current_a);
}

bool BluedInputSource::readOnce(std::ifstream &input_stream, DefaultDataManager &mgr) {
    const unsigned int buffer_size = 10000;
    static DataPoint buffer[buffer_size];
    unsigned int i = 0;
    for (; i < buffer_size; ++i) {
        if (!(input_stream.good() && this->continue_reading)) {
            i -= 1;
            return false;

        }
        buffer[i] = BluedInputSource::matchLine(input_stream);
    }
    mgr.addDataPoints(buffer, buffer + i);
    return true;
}


bool BluedInputSource::skipToData(std::ifstream &input_stream) {
    std::string line;
    while (input_stream.good()) {
        std::getline(input_stream, line);
        if (line == "X_Value,Current A,Current B,VoltageA,Comment\r") {
            return true;
        }
    }
    return false;
}

void BluedInputSource::readFile(const std::string &file_path, DefaultDataManager &mgr) {
    std::ifstream input_stream;
    input_stream.open(file_path, std::ifstream::in);

    if (!BluedInputSource::skipToData(input_stream)) {
        std::cerr << "The file could not be opened: " << file_path << std::endl;

        throw std::exception();
    }
    while (readOnce(input_stream, mgr)) {
        // do nothing
    }
}

