#ifndef SMART_SCREEN_ALGORITHMS_H
#define SMART_SCREEN_ALGORITHMS_H

#include <iostream>
#include <numeric>
#include <cmath>
#include <kiss_fft.h>
#include <vector>
#include <algorithm>
#include <exception>
#include <iostream>

#include "Utilities.h"

namespace Algorithms {
    template<typename IteratorType> auto
    rootMeanSquare(IteratorType begin, const IteratorType end) -> decltype(*begin) {
        decltype(*begin) result = 0.0;
        decltype(*begin) n = end - begin;
        while (begin != end) {
            result += *begin * *begin;
            ++begin;
        }
        return std::sqrt(result / n);
    }

    template<typename IteratorType> auto
    rootMeanSquareOfAmpere(IteratorType begin, const IteratorType end) -> decltype(begin->ampere()) {
        return rootMeanSquare(makeAmpereIterator(begin), makeAmpereIterator(end));
    }


    template<typename IteratorType, typename IteratorType2, typename DataType = float> DataType
    euclideanDistance(IteratorType begin, const IteratorType end, IteratorType2 begin2, const IteratorType2 end2) {
        DataType result = 0.0;
        DataType n = end - begin;
        while (begin != end && begin2 != end2) {
            DataType dim_diff = *begin - *begin2;
            result += dim_diff * dim_diff;
            ++begin;
            ++begin2;
        }
        return std::sqrt(result / n);
    }

    template<typename IteratorType, typename DataType = float> DataType
    mean(IteratorType begin, const IteratorType end) {
        DataType result = 0.0;
        DataType n = end - begin;

        result = std::accumulate(begin, end, 0.f);
        return result / n;
    }

    template<typename IteratorType, typename DataType = float> DataType
    variance(IteratorType begin, const IteratorType end) {
        DataType result = 0.0;
        DataType iter_mean = mean(begin, end);


        while (begin != end) {
            DataType diff = *begin - iter_mean;
            result += diff * diff;
            ++begin;
        }
        return result;
    }

    inline std::vector<float> getHarmonics(const std::vector<kiss_fft_cpx> &data_points, unsigned long base_frequency,
                                           unsigned long number_of_harmonics = 20, unsigned long search_radius = 5) {
        ++number_of_harmonics;
        assert(base_frequency * number_of_harmonics + search_radius < data_points.size());
        std::vector<float> result;

        for (unsigned long i = 2; i <= number_of_harmonics; ++i) {
            float harm = std::abs(std::max_element(data_points.begin() + i * base_frequency - search_radius,
                                          data_points.begin() + i * base_frequency + search_radius,
                                          [](const kiss_fft_cpx &cpx1, const kiss_fft_cpx &cpx2) {
                                              return std::abs(cpx1.r) < std::abs(cpx2.r);
                                          })->r);
            result.push_back(harm);
        }
        return result;
    }

}

#endif //SMART_SCREEN_ALGORITHMS_H
