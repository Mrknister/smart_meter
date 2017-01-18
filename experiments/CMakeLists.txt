cmake_minimum_required(VERSION 2.8)
project(experiments)

find_package(Boost COMPONENTS program_options REQUIRED)

set(experiment_deps dataloader libanalyze event_detector data_analyzer ${Boost_LIBRARIES})
include_directories(${Boost_INCLUDE_DIRS})

add_executable(simple_setup
    simple_setup/main.cpp
    )
add_executable(event_detection_setup
    event_detection_setup/main.cpp
    )
add_executable(event_classification_setup
    event_classification_setup/main.cpp
    event_classification_setup/CrossValidationResult.h
    event_classification_setup/SerializeEventLabelManager.h
    event_classification_setup/SelectPartitions.h)

add_executable(data_vis
    data_vis/main.cpp
    event_classification_setup/CrossValidationResult.h
    event_classification_setup/SerializeEventLabelManager.h
    data_vis/EventVisualizer.h
    data_vis/ClassificationVisualizer.h
    data_vis/CrossValidationStats.h)

add_executable(slimmed_validation
    event_classification_setup/slimmed_validation.cpp
    event_classification_setup/CrossValidationResult.h
    event_classification_setup/SerializeEventLabelManager.h
    )
add_executable(integrated_speed_setup
    integrated_speed_setup/main.cpp
    )

target_link_libraries(simple_setup ${experiment_deps})
target_link_libraries(event_detection_setup ${experiment_deps})
target_link_libraries(event_classification_setup ${experiment_deps})
target_link_libraries(data_vis ${experiment_deps})
target_include_directories(data_vis PRIVATE event_classification_setup)
target_link_libraries(slimmed_validation ${experiment_deps})
target_link_libraries(integrated_speed_setup ${experiment_deps})
