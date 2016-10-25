cmake_minimum_required(VERSION 2.8)
project (dataloader)


set (PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set (dataloader_INCLUDE_DIR ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})


#Project sources that will be built
FILE(GLOB_RECURSE DATALOADER_SRCS "${PROJECT_SOURCE_DIR}/*")

include(${CMAKE_SOURCE_DIR}/cmake/Scripts/AddDoxygenTarget.cmake)
add_doxygen_target("dataloader_doc")

# add ini file parser 
set(INIH_SOURCES 
    "${EXTERNAL_DEPENDENCIES_DIR}/inih/ini.c"
    )
include_directories("${EXTERNAL_DEPENDENCIES_DIR}/inih")
set(DATALOADER_SRCS
    ${DATALOADER_SRCS}
    ${INIH_SOURCES}
    )




find_package(SndFile REQUIRED)


add_library(${PROJECT_NAME}  ${DATALOADER_SRCS})
target_include_directories(${PROJECT_NAME} PUBLIC "${dataloader_INCLUDE_DIR}")

target_link_libraries(${PROJECT_NAME} -lm -lpthread)
target_link_libraries(${PROJECT_NAME} "${SNDFILE_LIBRARIES}")


install (TARGETS ${PROJECT_NAME} DESTINATION lib)
