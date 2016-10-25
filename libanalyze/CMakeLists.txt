cmake_minimum_required(VERSION 2.6)
project (analyze)


set (PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src)
set (PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
FILE(GLOB_RECURSE ANALYZE_SOURCES "${PROJECT_SOURCE_DIR}/*.c")



add_library(${PROJECT_NAME} ${ANALYZE_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_INCLUDE_DIR}")
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")


# add kiss_fft
add_library(kiss_fft ${EXTERNAL_DEPENDENCIES_DIR}/kiss_fft/kiss_fft.c)
target_include_directories(kiss_fft PUBLIC ${EXTERNAL_DEPENDENCIES_DIR}/kiss_fft/ BEFORE)
target_include_directories(${PROJECT_NAME}  PUBLIC ${EXTERNAL_DEPENDENCIES_DIR}/kiss_fft/ BEFORE)
target_link_libraries(${PROJECT_NAME} PUBLIC kiss_fft)



include(${CMAKE_SOURCE_DIR}/cmake/Scripts/AddDoxygenTarget.cmake)
add_doxygen_target("libanalyze_doc")



install(TARGETS ${PROJECT_NAME}
        INCLUDES DESTINATION include/
        ARCHIVE DESTINATION lib/
)
