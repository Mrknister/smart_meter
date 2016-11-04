
#ifndef _DATA_POINT_H_
#define _DATA_POINT_H_


/**
 * @brief This structure holds one datum for voltage and ampere power
 * 
 */
struct DefaultDataPoint {
    DefaultDataPoint(float volt_value, float ampere_value): volts(volt_value) , ampere(ampere_value) {}
    DefaultDataPoint(): volts(0) , ampere(0) {}
    DefaultDataPoint(const DefaultDataPoint& other) : volts(other.volts), ampere(other.ampere){}

    typedef float datum;
    datum volts;
    datum ampere;
};

#endif // _DATA_POINT_H_
