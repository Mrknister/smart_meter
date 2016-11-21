#ifndef SMART_SCREEN_EVENT_H
#define SMART_SCREEN_EVENT_H

#include <vector>
#include "EventMetaData.h"
#include "DefaultDataPoint.h"
#include "BluedDataPoint.h"
#include <boost/serialization/vector.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>

template<typename DataPointType> class Event {
public:
    std::vector<DataPointType> event_data;
    EventMetaData event_meta_data;

};




namespace boost {
    namespace serialization {

        template<class Archive>
        void serialize(Archive & ar, DefaultDataPoint & data_point, const unsigned int version)
        {
            ar & data_point.volts;
            ar & data_point.ampere;
        }

        template<class Archive>
        void serialize(Archive & ar, EventMetaData & meta_data, const unsigned int version)
        {
            ar & meta_data.event_time;
        }

        template<class Archive>
        void serialize(Archive & ar, Event<DefaultDataPoint> & event, const unsigned int version)
        {
            ar & event.event_data;
            ar & event.event_meta_data;
        }
        template<class Archive>
        void serialize(Archive & ar, Event<BluedDataPoint> & event, const unsigned int version)
        {
            ar & event.event_data;
            ar & event.event_meta_data;
        }
template<class Archive>
        void serialize(Archive & ar, BluedDataPoint & dp, const unsigned int version)
        {
            ar & dp.x_value;
            ar & dp.ampere;
            ar & dp.current_b;
            ar & dp.volts;
        }

    } // namespace serialization
} // namespace boost


#endif //SMART_SCREEN_EVENT_H
