
#if defined(__cplusplus)
extern "C" {
#endif

#include "fft_feature.h"

int fast_fourier_transform(FastFourierFeature* feature, float buffer[], int buffer_size, int offset)
{
    kiss_fft_cpx kiss_fft_buffer[DATA_POINTS_PER_FEATURE];
    size_t lenmem = 1024 + sizeof(FastFourierFeature);
    char cfg_buffer[lenmem];
    kiss_fft_cfg cfg =  kiss_fft_alloc(DATA_POINTS_PER_FEATURE, 0, (void*) cfg_buffer, &lenmem);


    int counter = 0;
    for (; counter < DATA_POINTS_PER_FEATURE; ++counter) {
        kiss_fft_buffer[counter].i = 0;
        kiss_fft_buffer[counter].r = buffer[(counter + offset) % buffer_size];
    }

    kiss_fft(cfg, kiss_fft_buffer, feature->data);

    return 1;
}


#if defined(__cplusplus)
} /* extern "C" */
#endif
