cmake_minimum_required(VERSION 2.8)
project(event_detector)

set(event_detector_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR})

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

include(${CMAKE_SOURCE_DIR}/cmake/Scripts/AddDoxygenTarget.cmake)
add_doxygen_target("event_detector_doc")


include_directories(${event_detector_INCLUDE_DIR})


add_executable(${PROJECT_NAME}
        src/EventDetector.cpp
        src/EventDetector.h
        src/EventMetaData.cpp
        src/EventMetaData.h
        src/EventStorage.cpp
        src/EventStorage.h
        src/main.cpp

        )
target_link_libraries(${PROJECT_NAME} analyze)
target_link_libraries(${PROJECT_NAME} dataloader)


install(TARGETS ${PROJECT_NAME} DESTINATION bin)
