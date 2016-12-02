#ifndef SMART_SCREEN_ALGORITHMS_H
#define SMART_SCREEN_ALGORITHMS_H

#include <iostream>
#include <numeric>
#include <cmath>


namespace Algorithms {

    template<typename IteratorType, typename DataType = float> DataType
    rootMeanSquare(IteratorType begin, const IteratorType end) {
        DataType result = 0.0;
        DataType n = end-begin;
        while (begin != end) {
            result += begin->ampere() * begin->ampere();
            ++begin;
        }
        return std::sqrt(result/n);
    }

    template<typename IteratorType, typename DataType = float> DataType
    wavelets(IteratorType begin, const IteratorType end) {
        DataType result = 0.0;
        DataType n = end-begin;
        while (begin != end) {
            result += begin->ampere() * begin->ampere();
            ++begin;
        }
        return std::sqrt(result/n);
    }
}

#endif //SMART_SCREEN_ALGORITHMS_H
