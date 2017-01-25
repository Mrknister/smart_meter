#include <iostream>

#define DONT_STORE_ANYTHING

#include <PowerMetaData.h>
#include <DynamicStreamMetaData.h>
#include <DefaultDataPoint.h>
#include <DefaultEventDetectionStrategy.h>
#include <EventDetector.h>
#include <chrono>
#include <DataClassifier.h>
#include <fstream>
#include <random>

using namespace std;

void fillBuffer(std::vector<DefaultDataPoint> &buffer, const PowerMetaData &conf);

vector<chrono::milliseconds>
fillDataQueue(AsyncDataQueue<DefaultDataPoint> *to_fill, PowerMetaData conf, std::chrono::milliseconds max_time_diff,
              unsigned long number_of_runs);

vector<chrono::milliseconds> fillDataQueue(AsyncDataQueue<DefaultDataPoint> *to_fill, PowerMetaData conf,
                                           std::chrono::milliseconds max_time_diff = std::chrono::milliseconds(1000));

vector<chrono::milliseconds>
fillDataQueue(AsyncDataQueue<DefaultDataPoint> *to_fill, PowerMetaData conf, std::chrono::milliseconds max_time_diff,
              unsigned long number_of_runs) {
    auto buffer_size = conf.sample_rate / 2;
    std::vector<DefaultDataPoint> buffer(buffer_size);
    std::vector<std::chrono::milliseconds> durations;
    durations.reserve(number_of_runs);
    fillBuffer(buffer, conf);
    for (unsigned long i = 0; i < number_of_runs; ++i) {
        auto time = std::chrono::system_clock::now();

        // insert half a second 2 times
        to_fill->addDataPoints(buffer.begin(), buffer.end());
        to_fill->addDataPoints(buffer.begin(), buffer.end());

        auto time_difference = std::chrono::system_clock::now() - time;
        durations.push_back(chrono::duration_cast<chrono::milliseconds>(time_difference));

        if (time_difference > max_time_diff) {
            cerr << "Time difference is bigger than the maximum allowed time difference: " << max_time_diff.count()
                 << " > " << chrono::duration_cast<chrono::milliseconds>(time_difference).count() << "\nfailed in run "
                 << i << " after processing " << buffer_size * i << " elements" << endl;
            throw std::exception();
        }
        if (i % 1000 == 0) {
            cout << "." << flush;
        }

    }
    cout << "\ndone" << endl;
    to_fill->notifyStreamEnd();
    return durations;

}

vector<chrono::milliseconds>
fillDataQueue(AsyncDataQueue<DefaultDataPoint> *to_fill, PowerMetaData conf, std::chrono::milliseconds max_time_diff) {
    unsigned long recommended_number_of_runs = (conf.max_data_points_in_queue / conf.sample_rate + 1) * 600;
    cout << "running the setup with " << recommended_number_of_runs << " buffer fills" << endl;
    return fillDataQueue(to_fill, conf, max_time_diff, recommended_number_of_runs);
}

void fillBuffer(std::vector<DefaultDataPoint> &buffer, const PowerMetaData &conf) {

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(-20.f, 20.f);
    for(auto& data_point: buffer ) {
        data_point.amps = dist(mt);
        data_point.volts = dist(mt);

    }
}

void storeDurationsToFile(const vector<chrono::milliseconds> &to_store, const string &file_name);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "usage speed_setup <config file> [<times file>] [<max duration per second in ms>]";
        return -1;
    }

    PowerMetaData conf;
    if (!conf.load(argv[1])) {
        std::cout << "Could not load config file: " << argv[1] << "\n";
        return -1;
    }
    chrono::milliseconds max_time_diff(1000);
    if(argc >= 4) {
        max_time_diff = chrono::milliseconds(atol(argv[3]));
    }


        std::cout << conf << endl;
    DynamicStreamMetaData stream_meta_data;
    stream_meta_data.setFixedPowerMetaData(conf);
    AsyncDataQueue<DefaultDataPoint> data_queue;
    data_queue.setQueueMaxSize(conf.max_data_points_in_queue);


    EventDetector<DefaultEventDetectionStrategy, DefaultDataPoint> detect;
    detect.startAnalyzing(&data_queue, &stream_meta_data, DefaultEventDetectionStrategy(-1000.0f));

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
    auto durations = fillDataQueue(&data_queue, conf, max_time_diff);
    detect.join();
    analyzer.stopAnalyzingWhenDone();
    cout << "maximum time taken to process data: " << std::max_element(durations.begin(), durations.end())->count()
         << endl;
    if (argc >= 3) {
        storeDurationsToFile(durations, argv[2]);
    }

}

void storeDurationsToFile(const vector<chrono::milliseconds> &to_store, const string &file_name) {
    ofstream out(file_name);
    if (out.bad()) {
        throw std::exception();
    }
    for (const auto &duration: to_store) {
        out << duration.count() << "\n";
    }
}


