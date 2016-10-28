#ifndef _BLUED_DATA_POINT_H_
#define _BLUED_DATA_POINT_H_


/**
 * @brief This structure holds one datum for voltage and ampere power
 * 
 */
struct BluedDataPoint {
    typedef float datum;

    constexpr BluedDataPoint(float x_value, float current_a, float current_b, float voltage_a) : x_value(x_value), current_a(current_a), current_b(current_b), voltage_a(voltage_a) {}

    constexpr BluedDataPoint() : x_value(0), current_a(0), current_b(0), voltage_a(0) {}

    constexpr BluedDataPoint(const BluedDataPoint &other) : x_value(other.x_value), current_a(other.current_a), current_b(other.current_b), voltage_a(other.voltage_a) {}

    float x_value;
    float current_a;
    float current_b;
    float voltage_a;

};

#endif // _DATA_POINT_H_
