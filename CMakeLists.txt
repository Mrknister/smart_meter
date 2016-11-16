cmake_minimum_required(VERSION 2.8)
project(smart_screen CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(EXTERNAL_DEPENDENCIES_DIR "${CMAKE_SOURCE_DIR}/external")
message("External dependency directory: ${EXTERNAL_DEPENDENCIES_DIR}")

set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_CXX_FLAGS "-std=c++17")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 ")
# could not use -Wsign-conversion because I have no idea to tell the compiler that it was actually intended.
#set(WARNINGS "-pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow  -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-unused")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g ${WARNINGS}")

add_custom_target(doc)

add_subdirectory("${CMAKE_SOURCE_DIR}/libanalyze")
add_subdirectory("${CMAKE_SOURCE_DIR}/dataloader")
add_subdirectory("${CMAKE_SOURCE_DIR}/event_detector/")

add_subdirectory (test)
enable_testing ()
add_test (NAME ENERGY_TESTS COMMAND Test)
