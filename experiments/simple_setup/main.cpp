#include <iostream>
#include <thread>
#define DEBUG_OUTPUT
#include "PowerMetaData.h"
#include "AsyncDataQueue.h"
#include "BluedHdf5InputSource.h"

#include "EventDetector.h"
#include "DataClassifier.h"
#include "Utilities.h"
int main(int argc, char **argv) {

    using namespace std;


    if (argc < 2) {
        std::cout << "Please pass the location of the config file.\n";
        return 0;
    }

    PowerMetaData conf;
    if (!conf.load(argv[1])) {
        std::cout << "Could not load config file: " << argv[1] << "\n";
        return -1;
    }

    std::cout << conf << endl;

    BluedHdf5InputSource data_source;
    data_source.data_manager.setQueueMaxSize(conf.max_data_points_in_queue);
    data_source.meta_data.setFixedPowerMetaData(conf);

    data_source.startReading(argv[2]);

    EventDetector<DefaultEventDetectionStrategy, BluedDataPoint> detect;
    detect.startAnalyzing(&data_source.data_manager, &data_source.meta_data, DefaultEventDetectionStrategy(1.0));

    DataClassifier<BluedDataPoint> analyzer;
    analyzer.startClassification(argv[3]);
    DataClassifier<BluedDataPoint>* analyzer_ptr = &analyzer;

    detect.storage.setEventStorageCallback([analyzer_ptr](Event<BluedDataPoint>& e) {
        analyzer_ptr->pushEvent(e);
    });

    detect.join();
    analyzer.stopAnalyzing();
    const static Eigen::IOFormat CSVFormat(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", "\n");
    Eigen::MatrixXf final_matrix = analyzer.getNormalizedLabeledMatrix();
    cout << "done analyzing\n\n\n" <<final_matrix.format(CSVFormat) <<"\n\n\n";

    for(auto& x: analyzer.getEventLabelManager().labeled_events) {
        std::cout << *x.event_meta_data.label<<std::endl;
    }



    return 0;
}
