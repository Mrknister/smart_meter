#include "LibAnalyzeTest.h"

#include <boost/test/unit_test.hpp>
#include <deque>
#include "Algorithms.h"
#include "DefaultDataPoint.h"

BOOST_AUTO_TEST_SUITE(libanalyze_suite)

    BOOST_AUTO_TEST_CASE(test_rms) {
        DefaultDataPoint test_points[] = {DefaultDataPoint(0, 1), DefaultDataPoint(0, 2), DefaultDataPoint(0, 2)};
        float result = Algorithms::rootMeanSquare(test_points, test_points + 3);
        float interval = 0.001;
        BOOST_TEST(result > std::sqrt(3) - interval / 2);
        BOOST_TEST(result < std::sqrt(3) + interval / 2);

    }

    BOOST_AUTO_TEST_CASE(test_deque) {
        using namespace std;
        deque<int> a{1,2,3};
        bool success = a.end() == a.begin() + 3;
        BOOST_TEST_MESSAGE("a == a " << success);
        a.erase(a.begin(), a.begin() + 3);

    }

BOOST_AUTO_TEST_SUITE_END()