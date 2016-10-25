cmake_minimum_required(VERSION 2.8)
project(smart_screen CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(EXTERNAL_DEPENDENCIES_DIR "${CMAKE_SOURCE_DIR}/external")

set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_CXX_FLAGS "-Wall -g -std=c++17")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 ")

set(CMAKE_CXX_FLAGS_DEBUG "-O0 ")

add_custom_target(doc)

add_subdirectory("${CMAKE_SOURCE_DIR}/libanalyze")
add_subdirectory("${CMAKE_SOURCE_DIR}/dataloader")
add_subdirectory("${CMAKE_SOURCE_DIR}/event_detector/")

add_subdirectory (test)
enable_testing ()
add_test (NAME ENERGY_TESTS COMMAND Test)
