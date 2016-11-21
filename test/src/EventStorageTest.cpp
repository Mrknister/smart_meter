#include "EventStorageTest.h"
#include <boost/test/unit_test.hpp>

#include "Algorithms.h"
#include "EventStorage.h"

BOOST_AUTO_TEST_SUITE(event_storage_suite)

    BOOST_AUTO_TEST_CASE(store_many_events) {
        EventStorage st;
        const int buffer_size = 2000;
        DefaultDataPoint buffer[buffer_size];
        const int files_created = 100;

        for(int i = 0; i<files_created; ++i ) {
            EventMetaData metaData;
            st.storeEvent(buffer,buffer + buffer_size,metaData);
        }
    }

BOOST_AUTO_TEST_SUITE_END()