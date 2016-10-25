#include "PowerMeterData.h"

#include <string.h>
#include <sstream>
#include <iostream>
#include <exception>


#include "ini.h"

template <typename T>
T lexical_cast(const char* str)
{
    T var;
    std::istringstream iss;
    iss.exceptions(std::istringstream::badbit | std::istringstream::failbit);
    iss.str(str);
    iss >> var;
    return var;
}


/**
 * @brief This function converts the value to the type T and stores it in the destination if it is the value we want to store.
 *
 * @param expected_section The ini section we expect.
 * @param expected_name The name of the attribute we expect.
 * @param section The actual section.
 * @param name The actual name.
 * @param value The value of the attribute.
 * @param destination The destination where we want to store our attribute.
 * @return Returns true if this is the attribute we expect, false otherwise.
 *
 * @throws Exception An exception if the reading fails
 *
 */
template <typename T> bool mapSetting(const char* expected_section, const char* expected_name, const char* section, const char* name, const char* value, T& destination) {
    if (strcmp(name, expected_name) != 0) {
        return false;
    }
    if (strcmp(section, expected_section) != 0) {
        return false;
    }
    destination = lexical_cast<T>(value);
    return true;
}

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    try {
        PowerMeterData* calib = (PowerMeterData*) user;

        bool success = mapSetting<float>("main", "scale_amps", section, name,value, calib->scale_amps);
        return success;
    } catch(std::exception& e) {
        return false;
    }
}

bool PowerMeterData::load(const std::string& file_path)
{
    if (ini_parse(file_path.c_str(), handler, this) < 0) {
        std::cout << "Could not load config." << std::endl;
        return false;
    }
    return true;
}



