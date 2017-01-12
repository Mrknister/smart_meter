#ifndef SMART_SCREEN_DATAANALYZER_H
#define SMART_SCREEN_DATAANALYZER_H

#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "EventFeatures.h"
#include "Algorithms.h"
#include "EventLabelManager.h"
#include "ClassificationConfig.h"
#include "FeatureExtractor.h"
#include <nabo/nabo.h>
#include <boost/optional/optional_io.hpp>
#include "EventStorage.h"


struct DefaultDataPoint;

template<typename DataPointType = DefaultDataPoint> class DataClassifier {
public:
    typedef float DataFeatureType;

    void
    startClassification(const std::string &label_location, const ClassificationConfig &config = ClassificationConfig());

    void startClassification(const ClassificationConfig &config = ClassificationConfig());

    void stopAnalyzing();

    void stopAnalyzingWhenDone();


    void pushEvent(const Event<DataPointType> &e);

    void classifyOneEvent(const EventFeatures &e);

    void addLabel(const LabelTimePair &label);


    void addLabels(const std::vector<LabelTimePair> &labels);

    void join() {
        if (this->runner.joinable())
            this->runner.join();
    }

    ~DataClassifier() { this->stopAnalyzing(); }

    Eigen::MatrixXf getNormalizedLabeledMatrix();

    EventLabelManager<DataPointType> getEventLabelManager();

    void setEventLabelManager(EventLabelManager<DataPointType> label_manager);
    std::size_t getNumberOfElementsOnStack();


private:
    void run();

    void processOneEvent(const Event<DataPointType> &features);

    Eigen::VectorXf convertToEigenVector(const EventFeatures &features);

    void regenerateMatrix();

    void addEventToNormalizedMatrix(const EventFeatures &features);

    bool needToRegenerateMatrixForVector(const Eigen::VectorXf &vec);

    bool featureVectorIsInRange(const Eigen::VectorXf &vec);

    void generateNormalizationVectors(const Eigen::MatrixXf &matrix);

    void generateRescaleNormalizationVectors(const Eigen::MatrixXf &matrix);

    void generateStandardizeNormalizationVectors(const Eigen::MatrixXf &matrix);

    void generateFirstNormalizationVectors(const Eigen::MatrixXf &matrix);

    std::vector<DataFeatureType> eigenToStdVector(const Eigen::VectorXf &vec) {
        std::vector<DataClassifier::DataFeatureType> result;
        result.reserve(vec.size());
        for (int i = 0; i < vec.size(); ++i) {
            result.push_back(vec(i));
        }
        return result;

    }


    Eigen::VectorXf generateMaxVector(const Eigen::MatrixXf &matrix);

    Eigen::VectorXf generateMinVector(const Eigen::MatrixXf &matrix);

    void pushToMatrix(const Eigen::VectorXf &vec);

    Eigen::VectorXf normalizeEvent(Eigen::VectorXf vec);

    void normalizeMatrix(Eigen::MatrixXf &matrix);



private:
    EventLabelManager<DataPointType> event_label_manager;
    ClassificationConfig classification_config;
    FeatureExtractor feature_extractor;
    std::thread runner;
    bool continue_analyzing = true;
    std::mutex events_mutex;
    std::condition_variable events_empty_variable;
    std::vector<Event<DataPointType>> events;
    Eigen::MatrixXf labeled_matrix;
    Eigen::VectorXf normalization_mul_vector;
    Eigen::VectorXf normalization_add_vector;
    std::unique_ptr<Nabo::NNSearchF> nns;


};


template<typename DataPointType> void DataClassifier<DataPointType>::pushEvent(const Event<DataPointType> &e) {
    {
        std::unique_lock<std::mutex> events_lock(events_mutex);
        events.push_back(e);
    }
    events_empty_variable.notify_one();
}

template<typename DataPointType> void
DataClassifier<DataPointType>::processOneEvent(const Event<DataPointType> &event) {
    EventFeatures features = feature_extractor.extractFeatures(event);

    // temporary hack... yeah like that's gonna be removed any time soon
    EventStorage<DataPointType> storage;
    storage.storeFeatureVector(features.feature_vector.begin(), features.feature_vector.end(),
                               features.event_meta_data.event_id);
    // hack end


    if (event_label_manager.findLabelAndAddEvent(features)) {
        this->addEventToNormalizedMatrix(features);
    } else {
        this->classifyOneEvent(features);
    }
}

template<typename DataPointType> void DataClassifier<DataPointType>::classifyOneEvent(const EventFeatures &features) {
    const unsigned long k = 5;
    if (this->event_label_manager.labeled_events.size() <= k) {
#ifdef DEBUG_OUTPUT
        std::cout << "not enough elements in cloud yet: " << this->event_label_manager.labeled_events.size() << "\n";
#endif
        return;
    }
    using namespace Eigen;
    using namespace Nabo;
#ifdef DEBUG_OUTPUT
    std::cout << "Classifying event:\n";
#endif
    VectorXi indices;
    VectorXf dists2;

    // Look for the 5 nearest neighbours of each query point,
    // We do not want approximations but we want to sort by the distance,
    indices.resize(k);
    dists2.resize(k);
    auto vect = this->convertToEigenVector(features);
    nns->knn(normalizeEvent(vect), indices, dists2, k);
#ifdef DEBUG_OUTPUT
    std::cout << "The labels of the closest neighbors are: ";
#endif

    std::map<EventMetaData::LabelType, int> labels;
    for (int i = 0; i < k; ++i) {
        labels[event_label_manager.labeled_events[indices(i)].event_meta_data.label.value()]++;
#ifdef DEBUG_OUTPUT
        std::cout << event_label_manager.labeled_events[indices(i)].event_meta_data.label;
#endif
    }
    int max = -1;
    EventMetaData::LabelType max_label;
    for (auto &x: labels) {
        if (x.second > max) {
            max = x.second;
            max_label = x.first;
        }
    }
    this->event_label_manager.addClassifiedEvent(features, max_label);
#ifdef DEBUG_OUTPUT
    std::cout << std::endl;
#endif

}


template<typename DataPointType> void
DataClassifier<DataPointType>::startClassification(const std::string &label_location,
                                                   const ClassificationConfig &config) {
    this->join();
    this->event_label_manager.loadLabelsFromFile(label_location);
    this->startClassification(config);
}

template<typename DataPointType> void DataClassifier<DataPointType>::stopAnalyzing() {
    this->continue_analyzing = false;
    this->events_empty_variable.notify_all();
    this->join();

}

template<typename DataPointType> void DataClassifier<DataPointType>::stopAnalyzingWhenDone() {
    {
        std::unique_lock<std::mutex> events_lock(this->events_mutex);
        this->events_empty_variable.wait(events_lock, [this]() {
            return this->events.empty();
        });
    }

    this->stopAnalyzing();

}

template<typename DataPointType> Eigen::VectorXf
DataClassifier<DataPointType>::convertToEigenVector(const EventFeatures &features) {
    Eigen::VectorXf result;
    result.resize(features.feature_vector.size());
    int count = 0;
    for (auto &f:features.feature_vector) {
        result(count) = f;
        ++count;
    }

    return result;
}

template<typename DataPointType> void
DataClassifier<DataPointType>::addEventToNormalizedMatrix(const EventFeatures &features) {


    Eigen::VectorXf feature_vec = convertToEigenVector(features);
    if (needToRegenerateMatrixForVector(feature_vec)) {

        regenerateMatrix();

    } else {
        pushToMatrix(normalizeEvent(feature_vec));
    }


}

template<typename DataPointType> bool
DataClassifier<DataPointType>::featureVectorIsInRange(const Eigen::VectorXf &vec) {
    if (this->normalization_add_vector.size() <= 0) {
        return false;
    }

    Eigen::VectorXf v = this->normalizeEvent(Eigen::VectorXf(vec));
    for (int i = 0; i < v.size(); ++i) {
        if (v(i) < -1.0 || v(i) > 1.0) {
            return false;
        }
    }

    return true;
}

template<typename DataPointType> void
DataClassifier<DataPointType>::generateNormalizationVectors(const Eigen::MatrixXf &matrix) {
    if (this->event_label_manager.labeled_events.size() == 1) {
        generateFirstNormalizationVectors(matrix);
    } else if (classification_config.normalization_mode == NormalizationMode::Rescale) {
        generateRescaleNormalizationVectors(matrix);
    } else {
        generateStandardizeNormalizationVectors(matrix);
    }

}

template<typename DataPointType> void
DataClassifier<DataPointType>::generateRescaleNormalizationVectors(const Eigen::MatrixXf &matrix) {

    Eigen::VectorXf max_vector;
    Eigen::VectorXf min_vector;
    min_vector = generateMinVector(matrix);
    max_vector = generateMaxVector(matrix);


    for (long i = 0; i < normalization_mul_vector.size(); ++i) {
        normalization_mul_vector(i) = 2.0f / (max_vector(i) - min_vector(i));
    }
    for (long i = 0; i < normalization_mul_vector.size(); ++i) {
        this->normalization_add_vector(i) = -1.0f * min_vector(i) * this->normalization_mul_vector(i) - 1.0f;
    }

}

template<typename DataPointType> void
DataClassifier<DataPointType>::generateStandardizeNormalizationVectors(const Eigen::MatrixXf &matrix) {
    if (matrix.cols() < 2) {
        return;
    }

    normalization_mul_vector.resize(matrix.rows());
    normalization_add_vector.resize(matrix.rows());
    const float *data = matrix.row(0).data();


    for (long i = 0; i < matrix.rows(); ++i) {
        auto row_vector = eigenToStdVector(matrix.row(i));

        FeatureExtractor::FeatureType mean = Algorithms::mean(row_vector.begin(), row_vector.end());


        FeatureExtractor::FeatureType variance = Algorithms::variance(row_vector.begin(), row_vector.end());

        FeatureExtractor::FeatureType deviation = std::sqrt(variance);
        normalization_mul_vector(i) = 1.f / deviation;
        normalization_add_vector(i) = -mean / deviation;
    }
}

template<typename DataPointType> Eigen::VectorXf
DataClassifier<DataPointType>::generateMaxVector(const Eigen::MatrixXf &matrix) {
    Eigen::VectorXf max = matrix.col(0);
    for (long i = 1; i < matrix.cols(); ++i) {
        for (long j = 0; j < matrix.rows(); ++j) {
            if (matrix(j, i) > max(j)) {
                max(j) = matrix(j, i);
            }
        }
    }
    return max;
}


template<typename DataPointType> Eigen::VectorXf
DataClassifier<DataPointType>::generateMinVector(const Eigen::MatrixXf &matrix) {
    Eigen::VectorXf min = matrix.col(0);
    for (long i = 1; i < matrix.cols(); ++i) {
        for (long j = 0; j < matrix.rows(); ++j) {
            if (matrix(j, i) < min(j)) {
                min(j) = matrix(j, i);
            }
        }
    }
    return min;
}

template<typename DataPointType> void DataClassifier<DataPointType>::pushToMatrix(const Eigen::VectorXf &vec) {
    long rows = vec.size();
    labeled_matrix.resize(rows, labeled_matrix.cols() + 1);
    labeled_matrix.col(labeled_matrix.cols() - 1) = vec;
}

template<typename DataPointType> void DataClassifier<DataPointType>::regenerateMatrix() {
    if (event_label_manager.labeled_events.empty()) {
        return;
    }
    long rows = event_label_manager.labeled_events.back().feature_vector.size();
    long cols = event_label_manager.labeled_events.size();

    this->labeled_matrix.resize(rows, cols);
    for (int i = 0; i < cols; ++i) {
        Eigen::VectorXf vec = convertToEigenVector(this->event_label_manager.labeled_events[i]);
        this->labeled_matrix.col(i) = vec;
    }

    this->generateNormalizationVectors(this->labeled_matrix);
    this->normalizeMatrix(this->labeled_matrix);
    nns = std::unique_ptr<Nabo::NNSearchF>(Nabo::NNSearchF::createKDTreeTreeHeap(this->labeled_matrix));
}

template<typename DataPointType> void DataClassifier<DataPointType>::normalizeMatrix(Eigen::MatrixXf &matrix) {
    const static Eigen::IOFormat CSVFormat(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", "\n");

    for (long i = 0; i < matrix.cols(); ++i) {
        matrix.col(i) = normalizeEvent(matrix.col(i));
    }

}

template<typename DataPointType> Eigen::VectorXf DataClassifier<DataPointType>::normalizeEvent(Eigen::VectorXf vec) {

    for (long j = 0; j < vec.size(); ++j) {
        vec(j) = vec(j) * this->normalization_mul_vector(j) + this->normalization_add_vector(j);
    }
    return vec;
}

template<typename DataPointType> bool
DataClassifier<DataPointType>::needToRegenerateMatrixForVector(const Eigen::VectorXf &vec) {
    return this->classification_config.normalization_mode != NormalizationMode::Rescale ||
           !featureVectorIsInRange(vec) || this->event_label_manager.labeled_events.size() <= 1;
}

template<typename DataPointType> void
DataClassifier<DataPointType>::generateFirstNormalizationVectors(const Eigen::MatrixXf &matrix) {
    normalization_mul_vector = matrix.col(0);
    normalization_mul_vector.setConstant(1.0);
    normalization_add_vector = -matrix.col(0);
}

template<typename DataPointType> void DataClassifier<DataPointType>::addLabel(const LabelTimePair &label) {
    std::lock_guard<std::mutex> lock(this->events_mutex);
    this->event_label_manager.addLabel(label);


}

template<typename DataPointType> void
DataClassifier<DataPointType>::addLabels(const std::vector<LabelTimePair> &labels) {

}

template<typename DataPointType> void
DataClassifier<DataPointType>::startClassification(const ClassificationConfig &config) {

    this->continue_analyzing = true;
    feature_extractor.setConfig(config);
    runner = std::thread(&DataClassifier<DataPointType>::run, this);
}

template<typename DataPointType> void DataClassifier<DataPointType>::run() {

    while (true) {
        // wait until an event is pushed
        std::unique_lock<std::mutex> events_lock(this->events_mutex);
        this->events_empty_variable.wait(events_lock, [this]() {
            return !this->events.empty() || !this->continue_analyzing;
        });
        // if we dont want to analyze events anymore, quit
        if (!this->continue_analyzing) {
            break;
        }
        this->processOneEvent(this->events.back());
        this->events.pop_back();
    }
}

template<typename DataPointType> Eigen::MatrixXf DataClassifier<DataPointType>::getNormalizedLabeledMatrix() {
    return this->labeled_matrix;
}

template<typename DataPointType> EventLabelManager<DataPointType>
DataClassifier<DataPointType>::getEventLabelManager() {

    std::lock_guard<std::mutex> events_lock(this->events_mutex);
    auto x = this->event_label_manager;
    return x;
}

template<typename DataPointType> void
DataClassifier<DataPointType>::setEventLabelManager(EventLabelManager<DataPointType> label_manager) {
    std::lock_guard<std::mutex> l(events_mutex);
    this->event_label_manager = label_manager;
    regenerateMatrix();
}

template<typename DataPointType> std::size_t DataClassifier<DataPointType>::getNumberOfElementsOnStack() {
    std::lock_guard<std::mutex> l(events_mutex);

    return events.size();
}


#endif //SMART_SCREEN_DATAANALYZER_H




























