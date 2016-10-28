#ifndef _TEST_H_
#define _TEST_H_

#include "DataPoint.h"
#include <vector>


bool vec_is_sorted(const std::vector<int>& vec) {
    int prev = vec.front();
    --prev;
    for(auto& dp: vec) {
        if(prev + 1 != dp) {
            return false;
        }
        prev = dp;
    }
    return true;
}


#endif
