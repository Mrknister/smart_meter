/** @file */
#include <string>
#include <ostream>


#ifndef _CONFIG_LOADER_H_
#define _CONFIG_LOADER_H_


/**
  *@brief The FileConfig holds all relevant meta data about our electrical data including sample rate, conversion factors and so on.
  */
struct PowerMetaData {
    /**
     * @brief Loads a file configuration into the attributes of this class.
     *
     * @return Returns false on failure.
     */
    bool load(const std::string &file_path);

    // attributes
    float scale_volts = 1.0; /**< The voltage values are multiplied by this factor. */
    float scale_amps = 1.0; /**< The ampere values are multiplied by this factor. */
    unsigned long sample_rate = 12000; /**< The sample rate of the current. 12000 for BLUED, 16000 for UK-DALE */
    unsigned long frequency = 50; /**< The frequency in Hz of the current. */
    float voltage = 220; /**< The voltage of the current. In the European  Union 220 V is the standard */
    std::string data_set_start_time = "";

    unsigned long max_data_points_in_queue = 16000; /**< The number of samples we store unitl the writing thread is blocked.*/
    int data_points_stored_of_event = 0; /**< The number of samples we store until the writing thread is blocked.*/
    int data_points_stored_before_event = 0;


    unsigned long dataPointsPerPeriod() const {
        return sample_rate / frequency;
    }


};

std::ostream &operator<<(std::ostream &stream, const PowerMetaData &meta_data);

#endif
