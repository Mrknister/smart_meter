#include "CrossValidationSuccess.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <Eigen/Eigen>
#include <set>

using namespace std;

void print_partition_overview(const std::vector<CrossValidationSuccess> &results);

void print_partition_overview_averages(const std::vector<CrossValidationSuccess> &results);

std::map<EventMetaData::LabelType, std::map<EventMetaData::LabelType, int>>
sumResults(const std::vector<CrossValidationSuccess> &results);

void print_label_averages(const std::vector<CrossValidationSuccess> &results);

void print_complete_results(const std::vector<CrossValidationSuccess> &results);

void remove_event_by_id(std::vector<CrossValidationSuccess> &results, EventMetaData::LabelType event_label);

void remove_event_by_id(CrossValidationSuccess &result, EventMetaData::LabelType event_label);

void print_ground_truth(const std::vector<CrossValidationSuccess> &results);

void print_confusion_matrix(const std::map<EventMetaData::LabelType, std::map<EventMetaData::LabelType, int>> &results);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "usage: results-analzer <archive file>\n";
        return -1;
    }
    ifstream archive_stream(argv[1]);
    boost::archive::text_iarchive archive(archive_stream);

    std::vector<CrossValidationSuccess> results;
    archive >> results;

    // with every event
    print_complete_results(results);
    print_partition_overview(results);
    print_label_averages(results);
    print_partition_overview_averages(results);
    print_confusion_matrix(sumResults(results));

}

void print_partition_overview(const std::vector<CrossValidationSuccess> &results) {
    cout << "Partition overview:\n\n";
    int count = 0;
    for (auto &result: results) {
        std::cout << "Patition " << count << ": correct classifications: " << result.classified_correctly
                  << "   incorrect classifications: " << result.classified_incorrectly << "\n";
        ++count;
    }
    cout << "\n\n\n";
}

void print_partition_overview_averages(const std::vector<CrossValidationSuccess> &results) {
    cout << "Average partition overview:\n\n";
    int count = 0;
    double correct = 0;
    double incorrect = 0;
    for (auto &result: results) {
        correct += result.classified_correctly;
        incorrect += result.classified_incorrectly;
        ++count;
    }
    cout << "average of correct classifications: " << correct / results.size()
         << "\naverage of incorrect classifications: " << incorrect / results.size() << "\n";
}

void print_label_averages(const std::vector<CrossValidationSuccess> &results) {
    auto sum = sumResults(results);
    cout << "Average overview broken down by label:\n\n";

    for (auto &map_brd: sum) {
        std::cout << "\nlabel " << map_brd.first << " was on average identified\n";

        int incorrect = 0;
        for (auto &pair:map_brd.second) {
            std::cout << static_cast<double>(pair.second) / results.size() << " times as " << pair.first << "\n";
            incorrect += pair.second;
        }

        std::cout << "average sum of correct detections: "
                  << static_cast<double>(map_brd.second[map_brd.first]) / results.size() << "\n";
        std::cout << "average sum of incorrect detections: "
                  << static_cast<double>(incorrect - map_brd.second[map_brd.first]) / results.size() << "\n";
    }
    cout << "\n\n\n";
}

void print_complete_results(const std::vector<CrossValidationSuccess> &results) {
    int count = 0;

    for (auto &result: results) {
        std::cout << "for partition " << count << " the following detection results were obtained: \n\n";
        for (auto &map_brd: result.broken_down) {
            std::cout << "label " << map_brd.first << " was identified\n";

            int incorrect = 0;
            for (auto &pair:map_brd.second) {
                std::cout << pair.second << " times as " << pair.first << "\n";
                incorrect += pair.second;
            }
            if (map_brd.second.find(map_brd.first) == map_brd.second.end()) {
                std::cout << "no incorrect detections\n";
            } else {
                std::cout << "sum of incorrect detections: " << incorrect - map_brd.second.at(map_brd.first) << "\n";
            }
        }
        ++count;
    }

    cout << "\n\n";
}

std::map<EventMetaData::LabelType, std::map<EventMetaData::LabelType, int>>
sumResults(const std::vector<CrossValidationSuccess> &results) {
    std::map<EventMetaData::LabelType, std::map<EventMetaData::LabelType, int>> sum;
    for (auto &result: results) {
        for (auto &map_brd: result.broken_down) {
            for (auto &pair:map_brd.second) {
                sum[map_brd.first][pair.first] += pair.second;
            }
        }
    }
    return sum;
}

void remove_event_by_id(std::vector<CrossValidationSuccess> &results, EventMetaData::LabelType event_label) {
    for (auto &partition: results) {
        remove_event_by_id(partition, event_label);
    }

}

void remove_event_by_id(CrossValidationSuccess &result, EventMetaData::LabelType event_label) {
    result.classified_correctly -= result.broken_down[event_label][event_label];
    result.classified_incorrectly += result.broken_down[event_label][event_label];
    for (auto &cl_id:result.broken_down[event_label]) {
        result.classified_incorrectly -= cl_id.second;
    }
    result.broken_down[event_label].clear();
    for (auto &label_results:result.broken_down) {
        for (auto &label_result: label_results.second) {
            if (label_results.first != label_result.first && label_result.first == event_label) {
                result.classified_incorrectly -= label_result.second;
                label_result.second = 0;
            }
        }
    }

}

void print_ground_truth(const std::vector<CrossValidationSuccess> &results) {
    for (auto &result: results) {
        for (auto &label_results:result.broken_down) {
            for (auto &label_result: label_results.second) {
                for (int i = 0; i < label_result.second; ++i)
                    cout << label_results.first << "," << label_result.first << endl;
            }
        }
    }
}

void
print_confusion_matrix(const std::map<EventMetaData::LabelType, std::map<EventMetaData::LabelType, int>> &results) {
    map<EventMetaData::LabelType, int> labels;
    std::set<EventMetaData::LabelType> tmp_labels;
    for (auto &label_results:results) {

        tmp_labels.insert(label_results.first);
        for (auto& pair:label_results.second) {
            tmp_labels.insert(pair.first);
        }
    }
    auto count = 1;

    auto mat_size = tmp_labels.size() + 1;

    Eigen::MatrixXd confusion_matrix(mat_size, mat_size);

    for (auto &label:tmp_labels) {
        confusion_matrix(0, count) = label;
        confusion_matrix(count, 0 ) = label;


        labels[label] = count;
        ++count;
    }


    for (auto &label_results:results) {
        auto pos = labels[label_results.first];

        for (auto &label_result: label_results.second) {

            auto row = labels[label_results.first];
            auto col = labels[label_result.first];
            confusion_matrix(row, col) = label_result.second;
        }
    }

    const static Eigen::IOFormat CSVFormat(Eigen::StreamPrecision, Eigen::DontAlignCols, " & ", "", "",
                                           "  \\\\ \\hline\n");

    cout << "confusion matrix: \n\n";


    cout << confusion_matrix.format(CSVFormat) << endl;

}
