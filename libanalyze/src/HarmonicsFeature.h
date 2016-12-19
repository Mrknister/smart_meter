#ifndef SMART_SCREEN_HARMONICSFEATURE_H
#define SMART_SCREEN_HARMONICSFEATURE_H

#include <kiss_fft.h>
#include <vector>
#include <algorithm>
#include <exception>
#include <iostream>


namespace Algorithms {

    std::vector<float> getHarmonics(const std::vector<kiss_fft_cpx>& data_points, long base_frequency, long number_of_harmonics = 20, long search_radius = 5) {
        if(base_frequency*number_of_harmonics+search_radius > data_points.size()) {
            std::cout << "Cannot compute that many harmonics. Base frequency: "<<base_frequency << "  number of harmonics: "
                      << number_of_harmonics
                      << "  number of data_points: " << data_points.size()
                      << "  number of data_points needed: " << base_frequency*number_of_harmonics+search_radius<< std::endl;
            //throw std::exception();
            return std::vector<float>();
        }
        std::vector<float> result;

        for (long i =0; i< number_of_harmonics; ++i) {
            float harm = std::max_element(data_points.begin()+i*base_frequency, data_points.begin()+i*base_frequency,
                                          [](const kiss_fft_cpx& cpx1, const kiss_fft_cpx& cpx2){
                                              return cpx1.r < cpx2.r;
                                          })->r;
            result.push_back(harm);
        }
        return result;
    }
}

#endif //SMART_SCREEN_HARMONICSFEATURE_H
