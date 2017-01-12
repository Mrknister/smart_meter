#include <iostream>
#include <thread>

#include "PowerMetaData.h"
#include "AsyncDataQueue.h"
#include "BluedHdf5InputSource.h"

#include "EventDetector.h"
#include "DataClassifier.h"

#include "CrossValidationSuccess.h"
#include "SerializeEventLabelManager.h"

void crossValidate(EventLabelManager<BluedDataPoint> &labeled_events, const std::string &result_file);

EventLabelManager<BluedDataPoint> initLabelManager(DataClassifier<BluedDataPoint> &classifier);

EventLabelManager<BluedDataPoint>
dropPartition(EventLabelManager<BluedDataPoint> labels, int total_number_of_partitions, int to_drop);

std::vector<EventFeatures>
getPartition(EventLabelManager<BluedDataPoint> labels, int total_number_of_partitions, int part_number);

CrossValidationSuccess
validatePartition(const EventLabelManager<BluedDataPoint> &labels, std::vector<EventFeatures> test_data);


int main(int argc, char **argv) {

    using namespace std;


    if (argc < 3) {
        cout << "usage: event_detection_setup <event file> <result file> [<events direcotry>]  [<feature vector archive>]\n";
        return 0;
    }
    EventStorage<BluedDataPoint> storage;
    if(argc >=4 ) {
        storage.event_directory = argv[3];
    }
    auto i = 0;

    DataClassifier<BluedDataPoint> analyzer;
    analyzer.startClassification(argv[1]);
    while (true) {
        try {

            auto event = storage.loadEvent(i);

            cout << "loaded event\n";
            analyzer.pushEvent(event);
            ++i;
        }
        catch (...) {
            break;
        }

    }


    analyzer.stopAnalyzingWhenDone();
    if(argc >=5 ) {
        ofstream out_stream(argv[4]);
        boost::archive::text_oarchive b_archive(out_stream);
        b_archive << analyzer.getEventLabelManager();
    }

    EventLabelManager<BluedDataPoint> labeled_events = initLabelManager(analyzer);

    crossValidate(labeled_events, argv[2]);

    return 0;
}

void crossValidate(EventLabelManager<BluedDataPoint> &labeled_events, const std::string &result_file) {
    int number_of_partitions = 10;
    std::vector<CrossValidationSuccess> results;
    for (int i = 0; i < number_of_partitions; ++i) {
        EventLabelManager<BluedDataPoint> training_set = dropPartition(labeled_events, number_of_partitions, i);
        std::vector<EventFeatures> test_set = getPartition(labeled_events, number_of_partitions, i);
        results.push_back(validatePartition(labeled_events, test_set));

    }
    for (auto &result: results) {
        std::cout << "correct classifications: " << result.classified_correctly << "   incorrect classifications: "
                  << result.classified_incorrectly << "\n";
    }
    int count = 0;
    std::map<EventMetaData::LabelType, double> averages;
    for (auto &result: results) {
        std::cout << "\n\nfor partition " << count << " the following detection results were obtained: \n";
        for (auto &map_brd: result.broken_down) {
            std::cout << "\nlabel " << map_brd.first << " was identified\n";

            int incorrect = 0;
            for(auto& pair:map_brd.second) {
                std::cout << pair.second << " times as " << pair.first << "\n";
                incorrect += pair.second;
            }
            std::cout << "sum of incorrect detections: " << incorrect - map_brd.second[map_brd.first];
        }
        ++count;
    }
    std::ofstream result_serialization_stream(result_file);

    boost::archive::text_oarchive ia(result_serialization_stream);
    ia << results;
}

EventLabelManager<BluedDataPoint> initLabelManager(DataClassifier<BluedDataPoint> &classifier) {
    EventLabelManager<BluedDataPoint> labeled_events = classifier.getEventLabelManager();
    const EventMetaData::LabelType not_an_event = 666;
    unsigned long unlabeled_pos = labeled_events.labeled_events.size();
    labeled_events.labeled_events.insert(labeled_events.labeled_events.end(), labeled_events.unlabeled_events.begin(),
                                         labeled_events.unlabeled_events.end());
    std::for_each(labeled_events.labeled_events.begin() + unlabeled_pos, labeled_events.labeled_events.end(),
                  [not_an_event](EventFeatures &f) {
                      f.event_meta_data.label = not_an_event;
                  });
    std::random_shuffle(labeled_events.labeled_events.begin(), labeled_events.labeled_events.end());
    labeled_events.labels = decltype(labeled_events.labels)();
    labeled_events.unlabeled_events.clear();
    return labeled_events;
}


EventLabelManager<BluedDataPoint>
dropPartition(EventLabelManager<BluedDataPoint> labels, int total_number_of_partitions, int to_drop) {
    assert(to_drop < total_number_of_partitions);
    unsigned long elements_per_partition = labels.labeled_events.size() / total_number_of_partitions;
    auto begin = labels.labeled_events.begin() + elements_per_partition * to_drop;
    unsigned long elements_to_drop_end = std::min(labels.labeled_events.size(), (to_drop + 1) * elements_per_partition);
    auto end = labels.labeled_events.begin() + elements_to_drop_end;
    auto r_end = labels.labeled_events.rbegin();
    if (to_drop != total_number_of_partitions - 1) {
        std::for_each(begin, end, [&labels, &r_end](EventFeatures &f) {
            std::swap(f, *r_end);
            ++r_end;
        });
    }
    labels.labeled_events.resize(labels.labeled_events.size() - elements_per_partition);
    return labels;
}

std::vector<EventFeatures>
getPartition(EventLabelManager<BluedDataPoint> labels, int total_number_of_partitions, int part_number) {

    assert(part_number < total_number_of_partitions);
    unsigned long elements_per_partition = labels.labeled_events.size() / total_number_of_partitions;
    auto begin = labels.labeled_events.begin() + elements_per_partition * part_number;
    unsigned long elements_to_fetch_end = std::min(labels.labeled_events.size(),
                                                   (part_number + 1) * elements_per_partition);
    auto end = labels.labeled_events.begin() + elements_to_fetch_end;
    return std::vector<EventFeatures>(begin, end);


}

CrossValidationSuccess
validatePartition(const EventLabelManager<BluedDataPoint> &labels, std::vector<EventFeatures> test_data) {
    DataClassifier<BluedDataPoint> test_classifier;
    test_classifier.setEventLabelManager(labels);
    for (auto &event:test_data) {
        test_classifier.classifyOneEvent(event);
    }
    std::vector<EventFeatures> classification_results = test_classifier.getEventLabelManager().unlabeled_events;
    int hit = 0;
    int miss = 0;
    assert(classification_results.size() == test_data.size());
    CrossValidationSuccess cvs;

    for (decltype(classification_results.size()) i = 0; i < classification_results.size(); ++i) {
        ++cvs.broken_down[*classification_results[i].event_meta_data.label][*test_data[i].event_meta_data.label];
        if (classification_results[i].event_meta_data.label == test_data[i].event_meta_data.label) {
            ++cvs.classified_correctly;
        } else {
            std::cout << "incorrectly classified " << *test_data[i].event_meta_data.label << " detected incorrect result of " <<  *classification_results[i].event_meta_data.label<<std::endl;
            ++cvs.classified_incorrectly;
        }
    }
    return cvs;

}
