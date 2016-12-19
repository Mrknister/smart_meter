#ifndef SMART_SCREEN_MEDALDATAPOINT_H
#define SMART_SCREEN_MEDALDATAPOINT_H

#include <exception>

#define NUMBER_OF_MEDAL_CHANNELS 6

struct MEDALDataPoint {

    typedef float datum;

    datum volts;
    datum currents[NUMBER_OF_MEDAL_CHANNELS];

    constexpr datum voltage() const { return volts; }

    constexpr void voltage(datum v) { this->volts = v; }

    constexpr datum ampere() const { return currents[0]; }

    void ampere(datum a) {
        throw std::exception(); // not to be used, u dimwit
    }

};

namespace boost {
    namespace serialization {

        template<class Archive> void serialize(Archive &ar, MEDALDataPoint &data_point, const unsigned int version) {
            ar & data_point.volts;
            ar & data_point.currents[0];
            ar & data_point.currents[1];
            ar & data_point.currents[2];
            ar & data_point.currents[3];
            ar & data_point.currents[4];
            ar & data_point.currents[5];
        }
        template<class Archive> void serialize(Archive &ar, Event<MEDALDataPoint> &event, const unsigned int version) {
            ar & event.event_data;
            ar & event.event_meta_data;
        }
    }
}


#endif //SMART_SCREEN_MEDALDATAPOINT_H
