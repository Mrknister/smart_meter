#ifndef SMART_SCREEN_ALGORITHMS_H
#define SMART_SCREEN_ALGORITHMS_H

#include <iostream>
#include <numeric>
#include <cmath>


namespace Algorithms {

    template<typename IteratorType, typename DataType = float> DataType
    rootMeanSquareOfAmpere(IteratorType begin, const IteratorType end) {
        DataType result = 0.0;
        DataType n = end-begin;
        while (begin != end) {
            result += begin->ampere() * begin->ampere();
            ++begin;
        }
        return std::sqrt(result/n);
    }

    template<typename IteratorType, typename DataType = float> DataType
    rootMeanSquare(IteratorType begin, const IteratorType end) {
        DataType result = 0.0;
        DataType n = end-begin;
        while (begin != end) {
            result += *begin * *begin;
            ++begin;
        }
        return std::sqrt(result/n);
    }


    template<typename IteratorType,typename IteratorType2, typename DataType = float> DataType
    euclideanDistance(IteratorType begin, const IteratorType end, IteratorType2 begin2, const IteratorType2 end2) {
        DataType result = 0.0;
        DataType n = end-begin;
        while (begin != end && begin2 != end2) {
            DataType dim_diff =*begin - *begin2;
            result += dim_diff*dim_diff;
            ++begin;
            ++begin2;
        }
        return std::sqrt(result/n);
    }
    template<typename IteratorType, typename DataType = float> DataType
    mean(IteratorType begin, const IteratorType end) {
        DataType result = 0.0;
        DataType n = end-begin;

        result = std::accumulate(begin,end,0.f);
        return result/n;
    }

    template<typename IteratorType, typename DataType = float> DataType
    variance(IteratorType begin, const IteratorType end) {
        DataType result = 0.0;
        DataType iter_mean = mean(begin, end);


        while (begin != end ) {
            DataType diff =*begin - iter_mean;
            result += diff*diff;
            ++begin;
        }
        return result;
    }

}

#endif //SMART_SCREEN_ALGORITHMS_H
