#include "LibAnalyzeTest.h"

#include <boost/test/unit_test.hpp>

#include "Algorithms.h"
#include "DefaultDataPoint.h"

BOOST_AUTO_TEST_SUITE(libanalyze_suite)

    BOOST_AUTO_TEST_CASE(test_rms) {
        DefaultDataPoint test_points[] = {DefaultDataPoint(0, 1), DefaultDataPoint(0, 2), DefaultDataPoint(0, 2)};
        float result = Algorithms::rootMeanSquare(test_points, test_points + 3);
        float interval=0.001;
        BOOST_TEST(result > std::sqrt(3)-interval/2);
        BOOST_TEST(result < std::sqrt(3)+interval/2);

    }

BOOST_AUTO_TEST_SUITE_END()