#ifndef SMART_SCREEN_FEATUREEXTRACTOR_H
#define SMART_SCREEN_FEATUREEXTRACTOR_H


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
    extractFFTFeatures(const Event<DataPointType> &event, std::vector<FeatureType> &feature_vec);

    void extractHarmonics(const std::vector<kiss_fft_cpx> &fft_of_current,
                          std::vector<FeatureExtractor::FeatureType> &feature_vec,
                          const EventMetaData &event_meta_data);

    template<typename DataPointType> void
    extractPhaseShift(const Event<DataPointType> &event, std::vector<FeatureExtractor::FeatureType> &feature_vec);

private:
    ClassificationConfig classification_config;
    FastFourierTransformCalculator fft_calculator;

};

template<typename DataPointType> EventFeatures FeatureExtractor::extractFeatures(const Event<DataPointType> &event) {
    std::vector<FeatureType> f_vect;
    extractRms<DataPointType>(event, f_vect);
    extractFFTFeatures<DataPointType>(event, f_vect);
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

template<typename DataPointType> void FeatureExtractor::extractFFTFeatures(const Event<DataPointType> &event,
                                                                           std::vector<FeatureExtractor::FeatureType> &feature_vec) {
    std::vector<kiss_fft_cpx> fft_ampere = fft_calculator.calculateAmpereFFT(event.event_begin(), event.event_end());
    std::vector<kiss_fft_cpx> fft_voltage = fft_calculator.calculateVoltageFFT(event.event_begin(), event.event_end());

    extractHarmonics(fft_ampere, feature_vec, event.event_meta_data);
    extractPhaseShift<DataPointType>(event,feature_vec);

}

void FeatureExtractor::extractHarmonics(const std::vector<kiss_fft_cpx> &fft_of_current,
                                        std::vector<FeatureExtractor::FeatureType> &feature_vec,
                                        const EventMetaData &event_meta_data) {
    std::vector<FeatureExtractor::FeatureType> harm = Algorithms::getHarmonics(fft_of_current,
                                                                               event_meta_data.power_meta_data.frequency,
                                                                               classification_config.number_of_harmonics,
                                                                               classification_config.harmonics_search_radius);
    std::transform(harm.begin(), harm.end(), harm.begin(), [](float f) { return std::abs(f); });
    feature_vec.insert(feature_vec.end(), harm.begin() + 1, harm.end());
}

template<typename DataPointType> void FeatureExtractor::extractPhaseShift(const Event<DataPointType> &event,
                                                                          std::vector<FeatureExtractor::FeatureType> &feature_vec) {


}


#endif //SMART_SCREEN_FEATUREEXTRACTOR_H
