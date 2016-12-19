cmake_minimum_required(VERSION 2.8)

project(energy_daq_inteface)


add_library(${PROJECT_NAME}
    src/daq_interface.h
    src/daq_interface.cpp
    src/MEDALDataPoint.h
    )


target_link_libraries(${PROJECT_NAME} dataloader event_detector data_analyzer)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/")

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
