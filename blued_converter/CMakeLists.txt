cmake_minimum_required(VERSION 2.8)
project(blued_converter)


add_executable(${PROJECT_NAME}
        src/BluedHdf5Converter.cpp
        src/BluedHdf5Converter.h
        src/main.cpp
        )

target_link_libraries(${PROJECT_NAME} dataloader libanalyze event_detector data_analyzer)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
