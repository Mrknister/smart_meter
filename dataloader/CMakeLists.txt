cmake_minimum_required(VERSION 3.0)
project(dataloader)


set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(dataloader_INCLUDE_DIR ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})


include(${CMAKE_SOURCE_DIR}/cmake/Scripts/AddDoxygenTarget.cmake)
add_doxygen_target("dataloader_doc")

# add ini file parser 
set(INIH_SOURCES
    "${EXTERNAL_DEPENDENCIES_DIR}/inih/ini.c"
    )
include_directories("${EXTERNAL_DEPENDENCIES_DIR}/inih")

set(DATALOADER_SRCS
    src/BluedDataPoint.h
    src/BluedDefinitions.h
    src/BluedHdf5Converter.cpp
    src/BluedHdf5Converter.h
    src/BluedHdf5InputSource.cpp
    src/BluedHdf5InputSource.h
    src/BluedInputSource.cpp
    src/BluedInputSource.h
    src/DataManager.h
    src/DefaultDataPoint.h
    src/PowerMetaData.cpp
    src/PowerMetaData.h
    src/BluedToDefaultDataManagerAdapter.h
    src/DynamicStreamMetaData.h
    src/DynamicStreamMetaData.cpp ../data_analyzer/src/DataAnalyzer.h)


find_package(SndFile REQUIRED)
find_package(Boost 1.60 COMPONENTS system filesystem date_time serialization REQUIRED)
find_package(HDF5 COMPONENTS CXX REQUIRED)


add_library(${PROJECT_NAME} ${DATALOADER_SRCS} ${INIH_SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
    "${dataloader_INCLUDE_DIR}"
    ${Boost_INCLUDE_DIRS}
    ${SNDFILE_INCLUDE_DIR}
    ${HDF5_INCLUDE_DIR}
    )


target_link_libraries(${PROJECT_NAME} -lm -lpthread)
target_link_libraries(${PROJECT_NAME}
    ${SNDFILE_LIBRARIES}
    ${Boost_LIBRARIES}
    ${HDF5_LIBRARIES}
    )
message("Boost Libraries linked to ${PROJECT_NAME}: ${Boost_LIBRARIES}")

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
