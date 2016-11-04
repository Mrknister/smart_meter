#ifndef _BLUED_DATA_POINT_H_
#define _BLUED_DATA_POINT_H_


/**
 * @brief This structure holds one datum for voltage and ampere power
 * 
 */
struct BluedDataPoint {
    typedef float datum;

    constexpr BluedDataPoint(float _x_value, float _current_a, float _current_b, float _voltage_a) : x_value(_x_value), current_a(_current_a), current_b(_current_b), voltage_a(_voltage_a) {}

    constexpr BluedDataPoint() : x_value(0), current_a(0), current_b(0), voltage_a(0) {}

    constexpr BluedDataPoint(const BluedDataPoint &other) : x_value(other.x_value), current_a(other.current_a), current_b(other.current_b), voltage_a(other.voltage_a) {}

    float x_value;
    float current_a;
    float current_b;
    float voltage_a;

};

#endif // _DATA_POINT_H_
