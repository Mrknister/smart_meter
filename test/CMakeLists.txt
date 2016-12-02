find_package(Boost REQUIRED COMPONENTS system filesystem unit_test_framework)

include_directories(
    ${Boost_INCLUDE_DIRS}
)

add_definitions(-DBOOST_TEST_DYN_LINK)
add_executable(Test
    src/DataLoaderTest.cpp
    src/DataLoaderTest.h
    src/LibAnalyzeTest.cpp
    src/LibAnalyzeTest.h
    src/EventStorageTest.cpp
    src/EventStorageTest.h
    src/test.cpp
    )

target_link_libraries(Test
    dataloader
    libanalyze
    event_detector
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    )
