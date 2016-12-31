#ifndef SMART_SCREEN_FEATUREEXTRACTOR_H
#define SMART_SCREEN_FEATUREEXTRACTOR_H

#include <complex>
#include "EventFeatures.h"
#include "FastFourierTransformCalculator.h"
#include "HarmonicsFeature.h"

class FeatureExtractor {
public:
    typedef EventFeatures::FeatureType FeatureType;

    template<typename DataPointType> EventFeatures extractFeatures(const Event<DataPointType> &event);

    void setConfig(ClassificationConfig config);

private:
    template<typename DataPointType> void
    extractRms(const Event<DataPointType> &event, std::vector<FeatureType> &feature_vec);


    template<typename DataPointType> void
    extractHarmonics(const Event<DataPointType> &event, std::vector<FeatureExtractor::FeatureType> &feature_vec);

    template<typename DataPointType> void
    extractPhaseShift(const Event<DataPointType> &event, std::vector<FeatureExtractor::FeatureType> &feature_vec);

    float calcPhaseShift(const std::vector<kiss_fft_cpx> &amps, const std::vector<kiss_fft_cpx> &volts,
                         unsigned long frequency);


private:
    ClassificationConfig classification_config;
    FastFourierTransformCalculator fft_calculator;

};

template<typename DataPointType> EventFeatures FeatureExtractor::extractFeatures(const Event<DataPointType> &event) {
    std::vector<FeatureType> f_vect;
    extractPhaseShift<DataPointType>(event, f_vect);

    extractRms<DataPointType>(event, f_vect);
    extractHarmonics<DataPointType>(event, f_vect);
    return EventFeatures(event.event_meta_data, f_vect);
}


template<typename DataPointType> void FeatureExtractor::extractRms(const Event<DataPointType> &event,
                                                                   std::vector<FeatureExtractor::FeatureType> &feature_vec) {
    FeatureType sub_rms = 0;
    unsigned long data_points_per_period = event.event_meta_data.power_meta_data.dataPointsPerPeriod();
    if (event.event_meta_data.power_meta_data.data_points_stored_before_event >= data_points_per_period) {
        sub_rms = Algorithms::rootMeanSquareOfAmpere(event.before_event_begin(),
                                                     event.before_event_begin() + data_points_per_period);
    }
    long loop_end = event.event_end() - event.event_begin() - data_points_per_period;
    auto begin = event.event_begin();
    int period_count = 0;
    for (long count = 0; count <= loop_end; count += data_points_per_period) {
        FeatureType rms = Algorithms::rootMeanSquareOfAmpere(begin, event.event_end());
        rms -= sub_rms;
        feature_vec.push_back(rms);
        begin += data_points_per_period;
        if (period_count >= classification_config.number_of_rms) break;
        ++period_count;
    }
}

void FeatureExtractor::setConfig(ClassificationConfig config) {
    this->classification_config = config;

}


template<typename DataPointType> void FeatureExtractor::extractHarmonics(const Event<DataPointType> &event,
                                                                         std::vector<FeatureExtractor::FeatureType> &feature_vec) {
    std::vector<kiss_fft_cpx> fft_ampere = fft_calculator.calculateAmpereFFT(event.event_begin(), event.event_end());
    std::vector<FeatureExtractor::FeatureType> harm = Algorithms::getHarmonics(fft_ampere,
                                                                               event.event_meta_data.power_meta_data.frequency,
                                                                               classification_config.number_of_harmonics,
                                                                               classification_config.harmonics_search_radius);
    std::transform(harm.begin(), harm.end(), harm.begin(), [](float f) { return std::abs(f); });
    feature_vec.insert(feature_vec.end(), harm.begin() + 1, harm.end());
}

template<typename DataPointType> void FeatureExtractor::extractPhaseShift(const Event<DataPointType> &event,
                                                                          std::vector<FeatureExtractor::FeatureType> &feature_vec) {
    long num_data_points = event.before_event_end() - event.before_event_begin();
    num_data_points = std::min(num_data_points, static_cast<long>(event.event_end() - event.event_begin()));

    std::vector<kiss_fft_cpx> fft_ampere_before = fft_calculator.calculateAmpereFFT(event.before_event_begin(),
                                                                                    event.before_event_begin() +
                                                                                    num_data_points);
    std::vector<kiss_fft_cpx> fft_voltage_before = fft_calculator.calculateVoltageFFT(event.before_event_begin(),
                                                                                      event.before_event_begin() +
                                                                                      num_data_points);
    float phase_shift_before = calcPhaseShift(fft_ampere_before, fft_voltage_before,
                                              event.event_meta_data.power_meta_data.frequency);

    std::vector<kiss_fft_cpx> fft_ampere = fft_calculator.calculateAmpereFFT(event.event_begin(),
                                                                             event.event_begin() + num_data_points);
    std::vector<kiss_fft_cpx> fft_voltage = fft_calculator.calculateVoltageFFT(event.event_begin(),
                                                                               event.event_begin() + num_data_points);
    float phase_shift = calcPhaseShift(fft_ampere, fft_voltage, event.event_meta_data.power_meta_data.frequency);

    float phase_shift_difference = phase_shift - phase_shift_before;
    feature_vec.push_back(phase_shift_difference);


}


float FeatureExtractor::calcPhaseShift(const std::vector<kiss_fft_cpx> &amps, const std::vector<kiss_fft_cpx> &volts,
                                       unsigned long frequency) {

    std::complex<float> amps_cpx(amps[frequency].r, amps[frequency].i);
    std::complex<float> volts_cpx(volts[frequency].r, volts[frequency].i);

    float angle_amps = std::arg(amps_cpx);
    float angle_volts = std::arg(volts_cpx);
    return  angle_amps - angle_volts;
}


#endif //SMART_SCREEN_FEATUREEXTRACTOR_H
