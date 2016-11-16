#ifndef _SMART_SCREEN_DYNAMICSTREAMMETADATA_H
#define _SMART_SCREEN_DYNAMICSTREAMMETADATA_H

#include <mutex>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/date_time.hpp>
#include "PowerMetaData.h"


class DynamicStreamMetaData {
public:
    typedef boost::multiprecision::cpp_int DataPointIdType;
    typedef boost::posix_time::ptime TimeType;
    typedef boost::posix_time::milliseconds MSDurationType;
    typedef boost::posix_time::microseconds USDurationType;


    DynamicStreamMetaData(DataPointIdType starting_package_id = 0) : synced_package_id(
            starting_package_id){}
    void setFixedPowerMetaData(PowerMetaData fixed_meta_data);
    PowerMetaData getFixedPowerMetaData() const;
    void syncTimePoint(DataPointIdType data_point_number, TimeType time);

    TimeType getDataPointTime(DataPointIdType data_point_number);


private:
    std::mutex sync_mutex;
    DataPointIdType synced_package_id;
    DynamicStreamMetaData::TimeType packet_time;
    PowerMetaData power_meta_data;

};


#endif //SMART_SCREEN_DYNAMICSTREAMMETADATA_H
