#ifndef _TEST_H_
#define _TEST_H_

#include "DataPoint.h"
#include <vector>


bool vec_is_sorted(const std::vector<DataPoint>& vec) {
    DataPoint prev = vec.front();
    prev.volts -= 1.0;
    for(auto& dp: vec) {
        if(prev.volts > dp.volts || prev.volts + 1.5 < dp.volts) {
            return false;
        }
        prev = dp;
    }
    return true;
}


#endif
