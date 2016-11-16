#ifndef SMART_SCREEN_FOURIERTRANSFORMCALCULATOR_H
#define SMART_SCREEN_FOURIERTRANSFORMCALCULATOR_H
#include <kiss_fft.h>
#include <memory>



class FastFourierTransformCalculator {
public:
    FastFourierTransformCalculator() {

    }
    FastFourierTransformCalculator(long _max_data_size){
        this->setMaxDataSetSize(_max_data_size);
    }
    void setMaxDataSetSize(long _max_data_size) {
        kiss_fft_buffer = std::unique_ptr<kiss_fft_cpx>(new kiss_fft_cpx[_max_data_size]);
        //long fft_result_buffer_size =

    }
    int calculate();

private:
    std::unique_ptr<kiss_fft_cpx> kiss_fft_buffer;
    long max_data_size = -1;
};

int FastFourierTransformCalculator::calculate() {/*
    size_t lenmem = 1024 + sizeof(FastFourierFeature);
    char cfg_buffer[lenmem];
    kiss_fft_cfg cfg =  kiss_fft_alloc(DATA_POINTS_PER_FEATURE, 0, (void*) cfg_buffer, &lenmem);


    int counter = 0;
    for (; counter < DATA_POINTS_PER_FEATURE; ++counter) {
        kiss_fft_buffer[counter].i = 0;
        kiss_fft_buffer[counter].r = buffer[(counter + offset) % buffer_size];
    }

    kiss_fft(cfg, kiss_fft_buffer, feature->data);
*/
    return 1;
}


#endif //SMART_SCREEN_FOURIERTRANSFORMCALCULATOR_H
