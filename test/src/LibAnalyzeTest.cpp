#include "LibAnalyzeTest.h"


#include <boost/test/unit_test.hpp>
#include <deque>
#include "Algorithms.h"
#include "DefaultDataPoint.h"
#define private public
#include "EventLabelManager.h"

BOOST_AUTO_TEST_SUITE(libanalyze_suite)

    BOOST_AUTO_TEST_CASE(test_rms) {
        DefaultDataPoint test_points[] = {DefaultDataPoint(0, 1), DefaultDataPoint(0, 2), DefaultDataPoint(0, 2)};
        float result = Algorithms::rootMeanSquare(test_points, test_points + 3);
        float interval = 0.001;
        BOOST_TEST(result > std::sqrt(3) - interval / 2);
        BOOST_TEST(result < std::sqrt(3) + interval / 2);

    }

    BOOST_AUTO_TEST_CASE(test_load_event_times) {
        EventLabelManager<DefaultDataPoint> events;
        events.loadLabelsFromFile("analyze_test_data/location_001_eventslist_A.txt");
        std::cout << "loaded " << events.labels.size() << " labels\n";
        for(auto l : events.labels) {
            std::cout << l.label << ", " << l.time << std::endl;
        }

    }


BOOST_AUTO_TEST_SUITE_END()