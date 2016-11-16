cmake_minimum_required(VERSION 2.8)
project(libanalyze)


set(libanalyze_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src)

set(ANALYZE_SOURCES
    src/Algorithms.h
    src/FastFourierTransformCalculator.h)


add_library(${PROJECT_NAME} ${ANALYZE_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${libanalyze_INCLUDE_DIR})
# because this is a header only library, cmake may not detect the linker language. Let's tell it explicitly that it's c++
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

## add gsl library
#find_package(GSL REQUIRED)
#target_include_directories(${PROJECT_NAME} PUBLIC ${GSL_INCLUDE_DIRS})
#target_link_libraries(${PROJECT_NAME} PUBLIC GSL_LIBRARIES)


# add kiss_fft
add_library(kiss_fft ${EXTERNAL_DEPENDENCIES_DIR}/kiss_fft/kiss_fft.c)
target_include_directories(kiss_fft PUBLIC "${EXTERNAL_DEPENDENCIES_DIR}/kiss_fft/")
target_link_libraries(${PROJECT_NAME} PUBLIC kiss_fft)

# Add a custom doxygen target for libanalyze
include(${CMAKE_SOURCE_DIR}/cmake/Scripts/AddDoxygenTarget.cmake)
add_doxygen_target("libanalyze_doc")


install(TARGETS ${PROJECT_NAME}
    INCLUDES DESTINATION include/
    ARCHIVE DESTINATION lib/)
