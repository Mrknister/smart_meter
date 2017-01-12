#ifndef SMART_SCREEN_EVENT_H
#define SMART_SCREEN_EVENT_H

#include <vector>
#include "EventMetaData.h"
#include "DefaultDataPoint.h"
#include "BluedDataPoint.h"
#include <boost/serialization/vector.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>

template<typename DataPointType> class Event {
public:
    std::vector<DataPointType> event_data;
    EventMetaData event_meta_data;
    typedef float DatumType;

    constexpr typename std::vector<DataPointType>::const_iterator before_event_begin() const {
        return event_data.begin();
    }

    constexpr typename std::vector<DataPointType>::const_iterator before_event_end() const {
        return event_begin();

    }

    constexpr typename std::vector<DataPointType>::const_iterator event_begin() const {
        return event_data.begin() + event_meta_data.power_meta_data.data_points_stored_before_event;
    }

    constexpr typename std::vector<DataPointType>::const_iterator event_end() const {
        return event_data.end();
    }

};


namespace boost {
    namespace serialization {

        template<class Archive> void serialize(Archive &ar, DefaultDataPoint &data_point, const unsigned int version) {
            ar & data_point.volts;
            ar & data_point.amps;
        }

        template<class Archive> void serialize(Archive &ar, EventMetaData &meta_data, const unsigned int version) {

            ar & meta_data.event_id;
            ar & meta_data.event_time;
            ar & meta_data.label;
            ar & meta_data.power_meta_data;

        }
        template<class Archive> void serialize(Archive &ar, PowerMetaData &meta_data, const unsigned int version) {
            ar & meta_data.scale_volts; /**< The voltage values are multiplied by this factor. */
            ar & meta_data.scale_amps; /**< The ampere values are multiplied by this factor. */
            ar & meta_data.sample_rate; /**< The sample rate of the current. 12000 for BLUED, 16000 for UK-DALE */
            ar & meta_data.frequency; /**< The frequency in Hz of the current. */
            ar & meta_data.voltage; /**< The voltage of the current. In the European  Union 220 V is the standard */
            ar & meta_data.data_set_start_time;

            ar & meta_data.max_data_points_in_queue; /**< The number of samples we store unitl the writing thread is blocked.*/
            ar & meta_data.data_points_stored_of_event; /**< The number of samples we store until the writing thread is blocked.*/
            ar & meta_data.data_points_stored_before_event;

        }

        template<class Archive> void
        serialize(Archive &ar, Event<DefaultDataPoint> &event, const unsigned int version) {
            ar & event.event_data;
            ar & event.event_meta_data;
        }

        template<class Archive> void serialize(Archive &ar, Event<BluedDataPoint> &event, const unsigned int version) {
            ar & event.event_data;
            ar & event.event_meta_data;
        }

        template<class Archive> void serialize(Archive &ar, BluedDataPoint &dp, const unsigned int version) {
            ar & dp.x_value;
            ar & dp.current_a;
            ar & dp.current_b;
            ar & dp.voltage_a;
        }

    } // namespace serialization
} // namespace boost


#endif //SMART_SCREEN_EVENT_H
