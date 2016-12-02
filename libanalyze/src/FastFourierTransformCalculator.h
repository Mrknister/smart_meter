#ifndef SMART_SCREEN_FOURIERTRANSFORMCALCULATOR_H
#define SMART_SCREEN_FOURIERTRANSFORMCALCULATOR_H

#include <kiss_fft.h>
#include <memory>
#include <vector>

#include "FastFourierFeature.h"


class FastFourierTransformCalculator {
public:
    FastFourierTransformCalculator() {

    }

    FastFourierTransformCalculator(unsigned long _max_data_size) {
        this->setMaxDataSetSize(_max_data_size);
    }

    void setMaxDataSetSize(unsigned long _max_data_size) {
        kiss_fft_buffer.resize(_max_data_size);
        size_t lenmem = 1024 + (_max_data_size) * sizeof(kiss_fft_cpx);
        cfg_buffer.resize(lenmem);

    }

    template<typename IteratorType> int calculate(IteratorType begin, IteratorType end);

private:
    std::vector<kiss_fft_cpx> kiss_fft_buffer;
    std::vector<char> cfg_buffer;
};

template<typename IteratorType> int FastFourierTransformCalculator::calculate(IteratorType begin, IteratorType end) {

    size_t lenmem = this->cfg_buffer.size();
    kiss_fft_cfg cfg = kiss_fft_alloc(static_cast<int>(kiss_fft_buffer.size()), 0,
                                      reinterpret_cast<void *>(cfg_buffer.data()), &lenmem);
    FastFourierFeature fft_f(end-begin);



    int counter = 0;
    while(begin!=end && counter < this->kiss_fft_buffer.size()) {
        kiss_fft_buffer[counter].i = 0;
        kiss_fft_buffer[counter].r = begin->ampere();
        ++begin;
        ++counter;
    }

    kiss_fft(cfg, kiss_fft_buffer.data(), fft_f.getBuffer());

    return 1;
}


#endif //SMART_SCREEN_FOURIERTRANSFORMCALCULATOR_H
