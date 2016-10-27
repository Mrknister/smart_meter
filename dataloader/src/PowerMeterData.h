/** @file */
#include <string>

#ifndef _CONFIG_LOADER_H_
#define _CONFIG_LOADER_H_


/**
  *@brief The FileConfig holds all relevant meta data about our electrical data including sample rate, conversion factors and so on.
  */
struct PowerMeterData {
    /**
     * @brief Loads a file configuration into the attributes of this class.
     *
     * @return Returns false on failure.
     */
    bool load(const std::string& file_path);

    // attributes
    float scale_volts = 1.0; /**< The voltage values are multiplied by this factor. */
    float scale_amps = 1.0; /**< The ampere values are multiplied by this factor. */
    unsigned int sample_rate = 12000;
    unsigned int max_data_points_in_queue = 16000;
    constexpr unsigned int dataPointsPerPeriod() const {
        return sample_rate/50;
    }


};

#endif
