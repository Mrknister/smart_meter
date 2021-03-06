cmake_minimum_required(VERSION 2.8)
project(event_detector)

set(event_detector_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR})

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include(${CMAKE_SOURCE_DIR}/cmake/Scripts/AddDoxygenTarget.cmake)
add_doxygen_target("event_detector_doc")
find_package(Boost COMPONENTS system filesystem date_time REQUIRED)




add_library(${PROJECT_NAME} STATIC
    src/EventDetector.h
    src/EventMetaData.h
    src/EventStorage.h
    src/DefaultEventDetectionStrategy.h
    src/dummy.cpp
    src/Event.h
    ../data_analyzer/src/EventFeatures.h)

target_link_libraries(${PROJECT_NAME} libanalyze)
target_link_libraries(${PROJECT_NAME} dataloader)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${event_detector_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    )


install(TARGETS ${PROJECT_NAME} DESTINATION bin)
