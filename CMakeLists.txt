cmake_minimum_required(VERSION 3.0)
project(smart_screen CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(EXTERNAL_DEPENDENCIES_DIR "${CMAKE_SOURCE_DIR}/external")
message("External dependency directory: ${EXTERNAL_DEPENDENCIES_DIR}")


#enable all warnings

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message("Using clang for building the project")
    # using Clang
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    message("Using gcc for building the project")

    # using GCC
    #  -Werror
    set(WARNINGS "-pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow  -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNINGS}")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    # using Intel C++
    # TODO enable intel warnings
    message("Using intel for building the project. This compiler is not supported. Proceed at your own risk.")

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # using Visual Studio C++
    message("Using visual studio c++ for building the project. This compiler is not supported. Proceed at your own risk.")

    if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else ()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif ()
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 ")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g ")

message("Building with the following compiler Flags: ${CMAKE_CXX_FLAGS}")

add_custom_target(doc)

add_subdirectory("${CMAKE_SOURCE_DIR}/libanalyze")
add_subdirectory("${CMAKE_SOURCE_DIR}/data_loader")
add_subdirectory("${CMAKE_SOURCE_DIR}/event_detector/")
add_subdirectory("${CMAKE_SOURCE_DIR}/data_analyzer/")
add_subdirectory("${CMAKE_SOURCE_DIR}/experiments/")
add_subdirectory("${CMAKE_SOURCE_DIR}/blued_converter/")
add_subdirectory("${CMAKE_SOURCE_DIR}/energy_daq_interface/")
add_subdirectory("${CMAKE_SOURCE_DIR}/energy_daq/")

