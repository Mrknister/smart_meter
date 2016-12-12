#ifndef _DATA_POINT_H_
#define _DATA_POINT_H_


/**
 * @brief This structure holds one datum for voltage and ampere power
 * 
 */
struct DefaultDataPoint {
    typedef float datum;

    constexpr DefaultDataPoint(float volt_value, float ampere_value) : volts(volt_value), amps(ampere_value) {}

    constexpr DefaultDataPoint() : volts(0), amps(0) {}


    constexpr datum voltage() const { return volts; }

    constexpr void voltage(datum v) { this->volts = v; }

    constexpr datum ampere() const { return amps; }

    constexpr void ampere(datum a) { this->amps = a; }

    datum volts;
    datum amps;
};

#endif // _DATA_POINT_H_
