#ifndef SMART_SCREEN_FOURIERTRANSFORMCALCULATOR_H
#define SMART_SCREEN_FOURIERTRANSFORMCALCULATOR_H

#include <kiss_fft.h>
#include <memory>
#include <vector>


class FastFourierTransformCalculator {
public:
    typedef float DataPointType;

    FastFourierTransformCalculator() {

    }

    FastFourierTransformCalculator(unsigned long _max_data_size) {
        this->setMaxDataSetSize(_max_data_size);
    }

    void setMaxDataSetSize(unsigned long _max_data_size) {
        max_data_size = _max_data_size;
        kiss_fft_buffer.resize(max_data_size);
        size_t lenmem = 1024 + (_max_data_size) * sizeof(kiss_fft_cpx);
        cfg_buffer.resize(lenmem);
    }


    template<typename IteratorType> std::vector<kiss_fft_cpx> calculateAmpereFFT(IteratorType begin, IteratorType end);

    template<typename IteratorType> std::vector<kiss_fft_cpx> calculateVoltageFFT(IteratorType begin, IteratorType end);


private:
    std::vector<kiss_fft_cpx> kiss_fft_buffer;

    unsigned long max_data_size = 0;
    std::vector<char> cfg_buffer;
};

template<typename IteratorType> std::vector<kiss_fft_cpx>
FastFourierTransformCalculator::calculateAmpereFFT(IteratorType begin, IteratorType end) {

    unsigned long points_to_compute = end - begin;
    if (points_to_compute > max_data_size) {
        this->setMaxDataSetSize(points_to_compute);
    }
    std::vector<kiss_fft_cpx> kiss_fft_result(points_to_compute);


    size_t lenmem = this->cfg_buffer.size();
    void *buffer = reinterpret_cast<void *>(cfg_buffer.data());

    kiss_fft_cfg cfg = kiss_fft_alloc(static_cast<int>(points_to_compute), 0, buffer, &lenmem);


    int counter = 0;
    while (begin != end && counter < kiss_fft_buffer.size()) {
        kiss_fft_buffer[counter].i = 0;
        kiss_fft_buffer[counter].r = begin->ampere();
        ++begin;
        ++counter;
    }
    auto bout_begin = kiss_fft_result.data();
    auto out_end = kiss_fft_result.data() + kiss_fft_result.size();

    kiss_fft(cfg, kiss_fft_buffer.data(), kiss_fft_result.data());
    return kiss_fft_result;
}

template<typename IteratorType> std::vector<kiss_fft_cpx>
FastFourierTransformCalculator::calculateVoltageFFT(IteratorType begin, IteratorType end) {

    unsigned long points_to_compute = end - begin;
    if (points_to_compute > max_data_size) {
        this->setMaxDataSetSize(points_to_compute);
    }
    std::vector<kiss_fft_cpx> kiss_fft_result(points_to_compute);


    size_t lenmem = this->cfg_buffer.size();
    kiss_fft_cfg cfg = kiss_fft_alloc(static_cast<int>(points_to_compute), 0,
                                      reinterpret_cast<void *>(cfg_buffer.data()), &lenmem);


    int counter = 0;
    while (begin != end && counter < kiss_fft_buffer.size()) {
        kiss_fft_buffer[counter].i = 0;
        kiss_fft_buffer[counter].r = begin->voltage();
        ++begin;
        ++counter;
    }

    kiss_fft(cfg, kiss_fft_buffer.data(), kiss_fft_result.data());
    return kiss_fft_result;
}

#endif //SMART_SCREEN_FOURIERTRANSFORMCALCULATOR_H
