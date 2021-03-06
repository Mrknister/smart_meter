/** @file */
#include <string>
#include <ostream>

#include <iostream>
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

    PowerMetaData() {}

    ~PowerMetaData() {}

    // Copy constructor needs more work
    PowerMetaData(const PowerMetaData &copy) {
        scale_volts = copy.scale_volts;
        scale_amps = copy.scale_amps;
        sample_rate = copy.sample_rate;
        frequency = copy.frequency;
        voltage = copy.voltage;
        data_set_start_time = std::string(copy.data_set_start_time);

        max_data_points_in_queue = copy.max_data_points_in_queue;
        data_points_stored_of_event = copy.data_points_stored_of_event;
        data_points_stored_before_event = copy.data_points_stored_before_event;
    }

    // Define assignment operator in terms of the copy constructor
    // Modified: There is a slight twist to the copy swap idiom, that you can
    //           Remove the manual copy made by passing the rhs by value thus
    //           providing an implicit copy generated by the compiler.
    PowerMetaData &operator=(PowerMetaData rhs) // Pass by value (thus generating a copy)
    {
        rhs.swap(*this); // Now swap data with the copy.
        // The rhs parameter will delete the array when it
        // goes out of scope at the end of the function
        return *this;
    }

    void swap(PowerMetaData &s) noexcept {
        std::swap(scale_volts, s.scale_volts);
        std::swap(scale_amps, s.scale_amps);
        std::swap(sample_rate, s.sample_rate);
        std::swap(frequency, s.frequency);
        std::swap(voltage, s.voltage);
        std::swap(data_set_start_time, s.data_set_start_time);

        std::swap(max_data_points_in_queue, s.max_data_points_in_queue);
        std::swap(data_points_stored_of_event, s.data_points_stored_of_event);
        std::swap(data_points_stored_before_event, s.data_points_stored_before_event);
    }

    unsigned long dataPointsPerPeriod() const {
        return sample_rate / frequency;
    }


};

std::ostream &operator<<(std::ostream &stream, const PowerMetaData &meta_data);

#endif
