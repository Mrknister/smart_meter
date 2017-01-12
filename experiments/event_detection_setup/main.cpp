#include <iostream>
#include <thread>
#define DEBUG_OUTPUT
#include "PowerMetaData.h"
#include "AsyncDataQueue.h"
#include "BluedHdf5InputSource.h"

#include "EventDetector.h"
#include "DataClassifier.h"
#include <atomic>

int main(int argc, char **argv) {

    using namespace std;

    if (argc < 5) {
        cout << "usage: event_detection_setup <config file> <data file> <event file> <threshold>\n";
        return 0;
    }

    PowerMetaData conf;
    if (!conf.load(argv[1])) {
        cout << "Could not load config file: " << argv[1] << "\n";
        return -1;
    }

    cout << conf << endl;
    cout << "threshold: " << std::stof(argv[4]) << "\n\n";

    BluedHdf5InputSource data_source;
    data_source.data_manager.setQueueMaxSize(conf.max_data_points_in_queue);
    data_source.meta_data.setFixedPowerMetaData(conf);

    data_source.startReading(argv[2]);

    EventDetector<DefaultEventDetectionStrategy, BluedDataPoint> detect;
    detect.startAnalyzing(&data_source.data_manager, &data_source.meta_data, DefaultEventDetectionStrategy(std::stof(argv[4])));

    EventLabelManager<> evl;
    evl.loadLabelsFromFile(argv[3]);
    mutex evl_mtx;

    detect.storage.setEventStorageCallback([&evl, &evl_mtx](Event<BluedDataPoint> &e) {
        lock_guard <mutex> evl_lck(evl_mtx);
        EventFeatures features(e.event_meta_data, vector<EventFeatures::FeatureType>());
        if (!evl.findLabelAndAddEvent(features)) {
            cout << "false pos\n";
            evl.addClassifiedEvent(features);
        } else {
            cout << "true pos\n";
        }
    });

    detect.join();
    cout << "true positives: " << evl.labeled_events.size() << endl;
    cout << "false positives: " << evl.unlabeled_events.size() << endl;
    cout << "false negatives: " << static_cast<long>(evl.labels.size()) - static_cast<long>(evl.labeled_events.size()) << endl;

    return 0;
}
