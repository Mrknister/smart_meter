
#ifndef _DATA_POINT_H_
#define _DATA_POINT_H_


/**
 * @brief This structure holds one datum for voltage and ampere power
 * 
 */
struct DefaultDataPoint {
    DefaultDataPoint(float volts, float ampere): volts(volts) , ampere(ampere) {}
    DefaultDataPoint(): volts(0) , ampere(0) {}
    DefaultDataPoint(const DefaultDataPoint& other) : volts(other.volts), ampere(other.ampere){}

    typedef float datum;
    datum volts;
    datum ampere;
};

#endif // _DATA_POINT_H_
