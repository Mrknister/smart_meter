#include <iostream>

#define DONT_STORE_ANYTHING

#include <PowerMetaData.h>
#include <DynamicStreamMetaData.h>
#include <DefaultDataPoint.h>
#include <DefaultEventDetectionStrategy.h>
#include <EventDetector.h>
#include <chrono>
#include <DataClassifier.h>

using namespace std;

void fillBuffer(std::vector<DefaultDataPoint> &buffer, const PowerMetaData &conf);

void
fillDataQueue(AsyncDataQueue<DefaultDataPoint> *to_fill, PowerMetaData conf, std::chrono::milliseconds max_time_diff,
              unsigned long number_of_runs);

void fillDataQueue(AsyncDataQueue<DefaultDataPoint> *to_fill, PowerMetaData conf,
                   std::chrono::milliseconds max_time_diff = std::chrono::milliseconds(1000));

void
fillDataQueue(AsyncDataQueue<DefaultDataPoint> *to_fill, PowerMetaData conf, std::chrono::milliseconds max_time_diff,
              unsigned long number_of_runs) {
    auto buffer_size = conf.sample_rate / 2;
    std::vector<DefaultDataPoint> buffer(buffer_size);

    for (unsigned long i = 0; i < number_of_runs; ++i) {
        auto time = std::chrono::system_clock::now();
        fillBuffer(buffer, conf);
        to_fill->addDataPoints(buffer.begin(), buffer.end());
        auto time_difference = std::chrono::system_clock::now() - time;

        if (time_difference > max_time_diff) {
            cerr << "Time difference is bigger than the maximum allowed time difference: " << max_time_diff.count()
                 << " < " << chrono::duration_cast<chrono::milliseconds>(time_difference).count() << "\nfailed in run "
                 << i << " after processing " << buffer_size * i << endl;
            throw std::exception();
        }
        if (i % 1000 == 0) {
            cout << "." << flush;
        }

    }
    cout << "\ndone" << endl;
    to_fill->notifyStreamEnd();

}

void
fillDataQueue(AsyncDataQueue<DefaultDataPoint> *to_fill, PowerMetaData conf, std::chrono::milliseconds max_time_diff) {
    unsigned long recommended_number_of_runs = (conf.max_data_points_in_queue / conf.sample_rate + 1) * 300;
    cout << "running the setup with " << recommended_number_of_runs << " buffer fills" << endl;
    fillDataQueue(to_fill, conf, max_time_diff, recommended_number_of_runs);
}

void fillBuffer(std::vector<DefaultDataPoint> &buffer, const PowerMetaData &conf) {

}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "usage speed_setup <config file>";
        return -1;
    }

    PowerMetaData conf;
    if (!conf.load(argv[1])) {
        std::cout << "Could not load config file: " << argv[1] << "\n";
        return -1;
    }

    std::cout << conf << endl;
    DynamicStreamMetaData stream_meta_data;
    stream_meta_data.setFixedPowerMetaData(conf);
    AsyncDataQueue<DefaultDataPoint> data_queue;
    data_queue.setQueueMaxSize(conf.max_data_points_in_queue);


    EventDetector<DefaultEventDetectionStrategy, DefaultDataPoint> detect;
    detect.startAnalyzing(&data_queue, &stream_meta_data, DefaultEventDetectionStrategy(-100.0f));

    DataClassifier<DefaultDataPoint> analyzer;
    analyzer.startClassification();
    DataClassifier<DefaultDataPoint> *analyzer_ptr = &analyzer;

    const size_t max_allowed_elements = 10;

    detect.storage.setEventStorageCallback([analyzer_ptr, max_allowed_elements](Event<DefaultDataPoint> &e) {
        analyzer_ptr->pushEvent(e);
        auto elements_on_stack = analyzer_ptr->getNumberOfElementsOnStack();
        if (elements_on_stack > max_allowed_elements) {
            std::cerr << "Too many elements on stack. The maximum amount tolerated is " << max_allowed_elements
                      << " there are currently " << elements_on_stack << " elements on the stack" << endl;
        }
    });
    fillDataQueue(&data_queue, conf);
    detect.join();
    analyzer.stopAnalyzingWhenDone();
}