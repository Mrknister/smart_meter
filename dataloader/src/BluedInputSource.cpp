#include "BluedInputSource.h"

#include <fstream>
#include <regex>
#include <iostream>



void BluedInputSource::startReading ( const std::string& file_path, DataManager& mgr )
{
    this->continue_reading = true;
    auto runner_function = std::bind ( & BluedInputSource::run, this, file_path, std::ref ( mgr ) );
    this->runner = std::thread ( runner_function );
    this->runner.detach();

}

void BluedInputSource::run ( const std::string& file_path, DataManager& mgr )
{
    std::ifstream input_stream;
    input_stream.open ( file_path,  std::ifstream::in );

    if ( !BluedInputSource::skipToData ( input_stream ) ) {
        throw std::exception();
    }
    while ( input_stream.good() ) {
        auto point = BluedInputSource::matchLine ( input_stream ) ;
        mgr.addDataPoint (point );
    }

}

DataPoint BluedInputSource::matchLine ( std::ifstream& input_stream )
{
    std::string line;
    char tmp_buffer[50];

    // read csv values
    float x_value;
    input_stream >> x_value;
    input_stream.getline(tmp_buffer, 50, ',' );

    float current_a;
    input_stream >> current_a;
    input_stream.getline(tmp_buffer, 50, ',' );

    float current_b;
    input_stream >> current_b;
    input_stream.getline(tmp_buffer, 50, ',' );

    float voltage_a;
    input_stream >> voltage_a;

    // skip to next line
    input_stream.getline(tmp_buffer, 50 );

    return DataPoint ( voltage_a, current_a );


}

bool BluedInputSource::skipToData ( std::ifstream& input_stream )
{
    std::string line;
    while ( input_stream.good() ) {
        std::getline ( input_stream, line );
        if ( line == "X_Value,Current A,Current B,VoltageA,Comment\r" ) {
            return true;
        }
    }
    return false;
}

