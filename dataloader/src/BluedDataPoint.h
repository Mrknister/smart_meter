#ifndef _BLUED_DATA_POINT_H_
#define _BLUED_DATA_POINT_H_


/**
 * @brief This structure holds one datum for voltage and ampere power
 * 
 */
struct BluedDataPoint {
    typedef float datum;

    constexpr BluedDataPoint(float _x_value, float _current_a, float _current_b, float _voltage_a) : x_value(_x_value), ampere(_current_a), current_b(_current_b), volts(_voltage_a) {}

    constexpr BluedDataPoint() : x_value(0), ampere(0), current_b(0), volts(0) {}

    constexpr BluedDataPoint(const BluedDataPoint &other) : x_value(other.x_value), ampere(other.ampere), current_b(other.current_b), volts(other.volts) {}

    float x_value;
    float ampere;
    float current_b;
    float volts;

};

#endif // _DATA_POINT_H_
