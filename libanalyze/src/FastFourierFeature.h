#ifndef SMART_SCREEN_FASTFOURIERFEATURE_H
#define SMART_SCREEN_FASTFOURIERFEATURE_H

#include <vector>

class FastFourierFeature {
public:
    FastFourierFeature(unsigned long num_data_points) {
        buffer.resize(num_data_points);
    }
    kiss_fft_cpx* getBuffer() {
        return buffer.data();
    }


private:
    std::vector<kiss_fft_cpx> buffer;
};
#endif //SMART_SCREEN_FASTFOURIERFEATURE_H
