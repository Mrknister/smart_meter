cmake_minimum_required(VERSION 2.8)
project(data_analyzer)

set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
include(${CMAKE_SOURCE_DIR}/cmake/Scripts/AddDoxygenTarget.cmake)

# include dlib as in project dependency
find_package(DLib REQUIRED)
add_doxygen_target("data_analyzer_doc")

add_library(${PROJECT_NAME}
    src/DataAnalyzer.h
    src/EventLabelManager.h)

target_link_libraries(${PROJECT_NAME} dataloader event_detector)
target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)


install(TARGETS ${PROJECT_NAME} DESTINATION bin)
