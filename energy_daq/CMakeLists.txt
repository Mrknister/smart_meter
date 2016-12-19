cmake_minimum_required(VERSION 2.8)
project(energy_daq)

find_package(PkgConfig)
pkg_check_modules(PC_LIBUSB REQUIRED libusb-1.0)
pkg_check_modules(PC_LIBFTDI REQUIRED libftdi1 )


add_executable(${PROJECT_NAME}
    src/main.c
    )


target_link_libraries(${PROJECT_NAME}
    ${PC_LIBUSB_LIBRARIES}
    ${PC_LIBFTDI_LIBRARIES}
    energy_daq_inteface
    -lm)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${PC_LIBFTDI_INCLUDE_DIRS})

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
