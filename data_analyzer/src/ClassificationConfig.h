#ifndef SMART_SCREEN_CLASSIFICATIONCONFIG_H
#define SMART_SCREEN_CLASSIFICATIONCONFIG_H

namespace NormalizationMode {
    enum NormalizationMode{
        Rescale,
        Standardize
    };
}
class ClassificationConfig {
public:
    int number_of_rms = 20;
    NormalizationMode::NormalizationMode normalization_mode = NormalizationMode::Standardize;
    long number_of_harmonics = 20;
    long harmonics_search_radius = 5;


};


#endif //SMART_SCREEN_CLASSIFICATIONCONFIG_H
