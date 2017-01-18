
#ifndef SMART_SCREEN_CROSSVALIDATIONSTATS_H
#define SMART_SCREEN_CROSSVALIDATIONSTATS_H

#include <vector>
#include <map>
#include "CrossValidationResult.h"

class CrossValidationStats {
public:
    void calculateStats(std::vector<CrossValidationResult> results);

    std::vector<Guess> correctClassifications(CrossValidationResult cv_result);

    std::vector<Guess> incorrectClassifications(CrossValidationResult cv_result);

    std::vector<Guess> correctClassifications();

    std::vector<Guess> incorrectClassifications();

    std::map<EventMetaData::LabelType, int> correctClassificationStats(const std::vector<Guess> &guesses);

    std::map<EventMetaData::LabelType, int> incorrectClassificationStats(const std::vector<Guess> &guesses);


private:
    std::vector<CrossValidationResult> results;
};

void CrossValidationStats::calculateStats(std::vector<CrossValidationResult> to_calc) {
    this->results = to_calc;
}

std::vector<Guess> CrossValidationStats::correctClassifications(CrossValidationResult cv_result) {
    std::vector<Guess> result;

    for (const auto &guess: cv_result.broken_down) {
        if (guess.classified_label == guess.actual_label) {
            result.push_back(guess);
        }
    }
    return result;
}

std::vector<Guess> CrossValidationStats::incorrectClassifications(CrossValidationResult cv_result) {
    std::vector<Guess> result;
    for (const auto &guess: cv_result.broken_down) {
        if (guess.classified_label != guess.actual_label) {
            result.push_back(guess);
        }
    }
    return result;
}

std::vector<Guess> CrossValidationStats::correctClassifications() {
    std::vector<Guess> result;
    for (const auto &cv_result: results) {
        auto tmp_class = correctClassifications(cv_result);
        result.insert(result.end(), tmp_class.begin(), tmp_class.end());
    }
    return result;
}


std::vector<Guess> CrossValidationStats::incorrectClassifications() {
    std::vector<Guess> result;
    for (const auto &cv_result: results) {
        auto tmp_class = incorrectClassifications(cv_result);
        result.insert(result.end(), tmp_class.begin(), tmp_class.end());
    }
    return result;
}

std::map<EventMetaData::LabelType, int>
CrossValidationStats::correctClassificationStats(const std::vector<Guess> &guesses) {
    std::map<EventMetaData::LabelType, int> stats;
    for (const auto &guess: guesses) {
        stats[guess.actual_label] += static_cast<int>(guess.actual_label == guess.classified_label);
    }
    return stats;
}

std::map<EventMetaData::LabelType, int>
CrossValidationStats::incorrectClassificationStats(const std::vector<Guess> &guesses) {
    std::map<EventMetaData::LabelType, int> stats;
    for (const auto &guess: guesses) {
        stats[guess.actual_label] += static_cast<int>(guess.actual_label == guess.classified_label);
    }
    return stats;
}


#endif //SMART_SCREEN_CROSSVALIDATIONSTATS_H
