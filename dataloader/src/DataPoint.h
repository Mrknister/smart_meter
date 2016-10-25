
#ifndef _DATA_POINT_H_
#define _DATA_POINT_H_


/**
 * @brief This structure holds one datum for voltage and ampere power
 * 
 */
struct DataPoint {
    DataPoint(float volts, float ampere): volts(volts) , ampere(ampere) {}
    DataPoint(): volts(0) , ampere(0) {}
    DataPoint(const DataPoint& other) : volts(other.volts), ampere(other.ampere){}

    typedef float datum;
    datum volts;
    datum ampere;
};

#endif // _DATA_POINT_H_
