cmake_minimum_required(VERSION 2.8)
project(event_detector)

set(event_detector_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR})

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include(${CMAKE_SOURCE_DIR}/cmake/Scripts/AddDoxygenTarget.cmake)
add_doxygen_target("event_detector_doc")
find_package(Boost COMPONENTS system filesystem date_time REQUIRED)



add_library(event_detection_lib STATIC
    src/EventDetector.h
    src/EventMetaData.h
    src/EventStorage.h
    src/DefaultEventDetectionStrategy.h
    src/dummy.cpp
    src/Event.h
    )

target_link_libraries(event_detection_lib libanalyze)
target_link_libraries(event_detection_lib dataloader)
target_link_libraries(event_detection_lib
    ${Boost_LIBRARIES}
    )
target_include_directories(event_detection_lib
    PUBLIC
    ${event_detector_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    )

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/Event.h)

target_link_libraries(${PROJECT_NAME} event_detection_lib)


install(TARGETS ${PROJECT_NAME} DESTINATION bin)
