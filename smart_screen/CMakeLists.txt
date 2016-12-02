cmake_minimum_required(VERSION 2.8)



add_executable(${PROJECT_NAME}
    src/main.cpp
    )

target_link_libraries(${PROJECT_NAME} dataloader libanalyze event_detector data_analyzer)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
