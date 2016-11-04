#ifndef _TEST_H_
#define _TEST_H_

#include "DefaultDataPoint.h"
#include <vector>
#include <string>


bool vec_is_sorted(const std::vector<int>& vec);
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
