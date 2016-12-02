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

};


namespace boost {
    namespace serialization {

        template<class Archive> void serialize(Archive &ar, DefaultDataPoint &data_point, const unsigned int version) {
            ar & data_point.volts;
            ar & data_point.amps;
        }

        template<class Archive> void serialize(Archive &ar, EventMetaData &meta_data, const unsigned int version) {
            ar & meta_data.event_time;
            ar & meta_data.label;
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
