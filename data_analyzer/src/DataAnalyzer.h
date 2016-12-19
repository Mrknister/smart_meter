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


struct DefaultDataPoint;

template<typename DataPointType = DefaultDataPoint> class DataAnalyzer {
public:
    typedef float DataFeatureType;

    void
    startClassification(const std::string &label_location, const ClassificationConfig &config = ClassificationConfig());

    void stopAnalyzing();

    void pushEvent(const Event<DataPointType> &e);

    void classifyOneEvent(const EventFeatures &e);

    void addLabel(const LabelTimePair &label);


    void addLabels(const std::vector<LabelTimePair> &labels);

    void join() {
        if (this->runner.joinable())
            this->runner.join();
    }

    ~DataAnalyzer() { this->stopAnalyzing(); }

private:
    void processOneEvent(const Event<DataPointType> &features);

    Eigen::VectorXf getFeatureVector(const EventFeatures &features);

    void regenerateMatrix();

    void addEventToNormalizedMatrix(const EventFeatures &features);

    bool needToRegenerateMatrixForVector(const Eigen::VectorXf &vec);

    bool featureVectorIsInRange(const Eigen::VectorXf &vec);

    void generateNormalizationVectors(const Eigen::MatrixXf &matrix);

    void generateRescaleNormalizationVectors(const Eigen::MatrixXf &matrix);

    void generateStandardizeNormalizationVectors(const Eigen::MatrixXf &matrix);

    void generateFirstNormalizationVectors(const Eigen::MatrixXf &matrix);

    std::vector<DataFeatureType> eigenToStdVector(const Eigen::VectorXf &vec){
        std::vector<DataAnalyzer::DataFeatureType> result;
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


};


template<typename DataPointType> void DataAnalyzer<DataPointType>::pushEvent(const Event<DataPointType> &e) {
    {
        std::unique_lock<std::mutex> events_lock(events_mutex);
        events.push_back(e);
    }
    events_empty_variable.notify_one();
}

template<typename DataPointType> void DataAnalyzer<DataPointType>::processOneEvent(const Event<DataPointType> &event) {
    std::cout << "processing event\n";
    EventFeatures features = feature_extractor.extractFeatures(event);

    if (event_label_manager.findLabelAndAddEvent(features)) {
        this->addEventToNormalizedMatrix(features);
    } else {

        this->classifyOneEvent(features);
    }
}

template<typename DataPointType> void DataAnalyzer<DataPointType>::classifyOneEvent(const EventFeatures &features) {
    const unsigned long k = 5;
    if (this->event_label_manager.labeled_events.size() <= k) {
        std::cout << "not enough elements in cloud yet: " << this->event_label_manager.labeled_events.size() << "\n";
        return;
    }
    using namespace Eigen;
    using namespace Nabo;
    std::cout << "Classifying event:\n";
    std::unique_ptr<NNSearchF> nns(NNSearchF::createKDTreeLinearHeap(this->labeled_matrix));
    VectorXi indices;
    VectorXf dists2;

    // Look for the 5 nearest neighbours of each query point,
    // We do not want approximations but we want to sort by the distance,
    indices.resize(k);
    dists2.resize(k);
    nns->knn(this->getFeatureVector(features), indices, dists2, k);

    std::cout << "The labels of the closest neighbors are: ";
    for (int i = 0; i < k; ++i) {
        std::cout << event_label_manager.labeled_events[indices(i)].event_meta_data.label;
    }
    std::cout << std::endl;

}


template<typename DataPointType> void
DataAnalyzer<DataPointType>::startClassification(const std::string &label_location,
                                                 const ClassificationConfig &config) {
    this->join();
    this->continue_analyzing = true;
    this->event_label_manager.loadLabelsFromFile(label_location);
    feature_extractor.setConfig(config);

    runner = std::thread([this]() {

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
    });
}

template<typename DataPointType> void DataAnalyzer<DataPointType>::stopAnalyzing() {
    this->continue_analyzing = false;
    this->events_empty_variable.notify_all();
    this->join();

}

template<typename DataPointType> Eigen::VectorXf
DataAnalyzer<DataPointType>::getFeatureVector(const EventFeatures &features) {
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
DataAnalyzer<DataPointType>::addEventToNormalizedMatrix(const EventFeatures &features) {


    Eigen::VectorXf feature_vec = getFeatureVector(features);
    std::cout << "feature_vec: " << feature_vec << "\n\n";
    if (needToRegenerateMatrixForVector(feature_vec)) {
        std::cout << "\nRegenerating matrix:\n" << std::endl;

        regenerateMatrix();

        std::cout << "normalization_add_vector:\n" << this->normalization_add_vector << "\n";
        std::cout << "normalization_mul_vector:\n" << this->normalization_mul_vector << "\n";
    } else {
        std::cout << "Is in range\n" << std::endl;
        pushToMatrix(normalizeEvent(feature_vec));
    }


}

template<typename DataPointType> bool DataAnalyzer<DataPointType>::featureVectorIsInRange(const Eigen::VectorXf &vec) {
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
DataAnalyzer<DataPointType>::generateNormalizationVectors(const Eigen::MatrixXf &matrix) {
    if (this->event_label_manager.labeled_events.size() == 1) {
        generateFirstNormalizationVectors(matrix);
    } else if (classification_config.normalization_mode == NormalizationMode::Rescale) {
        generateRescaleNormalizationVectors(matrix);
    } else {
        generateStandardizeNormalizationVectors(matrix);
    }

}

template<typename DataPointType> void
DataAnalyzer<DataPointType>::generateRescaleNormalizationVectors(const Eigen::MatrixXf &matrix) {

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
DataAnalyzer<DataPointType>::generateStandardizeNormalizationVectors(const Eigen::MatrixXf &matrix) {
    if (matrix.cols() < 2) {
        return;
    }

    normalization_mul_vector.resize(matrix.rows());
    normalization_add_vector.resize(matrix.rows());
    const float *data = matrix.row(0).data();


    for (long i = 0; i < matrix.rows(); ++i) {
        auto row_vector = eigenToStdVector(matrix.row(i));

        FeatureExtractor::FeatureType mean = Algorithms::mean(row_vector.begin(),
                                                              row_vector.end());


        FeatureExtractor::FeatureType variance = Algorithms::variance(row_vector.begin(),
                                                                      row_vector.end());

        FeatureExtractor::FeatureType deviation = std::sqrt(variance);
        normalization_mul_vector(i) = 1.f / deviation;
        normalization_add_vector(i) = -mean / deviation;
    }
}

template<typename DataPointType> Eigen::VectorXf
DataAnalyzer<DataPointType>::generateMaxVector(const Eigen::MatrixXf &matrix) {
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
DataAnalyzer<DataPointType>::generateMinVector(const Eigen::MatrixXf &matrix) {
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

template<typename DataPointType> void DataAnalyzer<DataPointType>::pushToMatrix(const Eigen::VectorXf &vec) {
    long rows = vec.size();
    labeled_matrix.resize(rows, labeled_matrix.cols() + 1);
    labeled_matrix.col(labeled_matrix.cols() - 1) = vec;
}

template<typename DataPointType> void DataAnalyzer<DataPointType>::regenerateMatrix() {
    long rows = event_label_manager.labeled_events.back().feature_vector.size();
    long cols = event_label_manager.labeled_events.size();

    this->labeled_matrix.resize(rows, cols);
    for (int i = 0; i < cols; ++i) {
        Eigen::VectorXf vec = getFeatureVector(this->event_label_manager.labeled_events[i]);
        this->labeled_matrix.col(i) = vec;
    }

    this->generateNormalizationVectors(this->labeled_matrix);
    this->normalizeMatrix(this->labeled_matrix);
}

template<typename DataPointType> void DataAnalyzer<DataPointType>::normalizeMatrix(Eigen::MatrixXf &matrix) {
    std::cout << "unnormalized matrix:\n" << matrix << "\n\n";

    for (long i = 0; i < matrix.cols(); ++i) {
        matrix.col(i) = normalizeEvent(matrix.col(i));
    }
    std::cout << "normalized matrix:\n" << matrix << "\n\n";

}

template<typename DataPointType> Eigen::VectorXf DataAnalyzer<DataPointType>::normalizeEvent(Eigen::VectorXf vec) {

    for (long j = 0; j < vec.size(); ++j) {
        vec(j) = vec(j) * this->normalization_mul_vector(j) + this->normalization_add_vector(j);
    }
    return vec;
}

template<typename DataPointType> bool
DataAnalyzer<DataPointType>::needToRegenerateMatrixForVector(const Eigen::VectorXf &vec) {
    return this->classification_config.normalization_mode != NormalizationMode::Rescale ||
           !featureVectorIsInRange(vec) || this->event_label_manager.labeled_events.size() <= 1;
}

template<typename DataPointType> void
DataAnalyzer<DataPointType>::generateFirstNormalizationVectors(const Eigen::MatrixXf &matrix) {
    normalization_mul_vector = matrix.col(0);
    normalization_mul_vector.setConstant(1.0);
    normalization_add_vector = -matrix.col(0);
}

template<typename DataPointType>void DataAnalyzer<DataPointType>::addLabel(const LabelTimePair &label) {
    std::lock_guard<std::mutex> lock(this->events_mutex);
    this->event_label_manager.addLabel(label);



}

template<typename DataPointType>void DataAnalyzer<DataPointType>::addLabels(const std::vector<LabelTimePair> &labels) {

}




#endif //SMART_SCREEN_DATAANALYZER_H




























