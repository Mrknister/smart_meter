cmake_minimum_required(VERSION 2.8)
project(experiments)


set(experiment_deps dataloader libanalyze event_detector data_analyzer)


add_executable(simple_setup
    simple_setup/main.cpp
    )
add_executable(event_detection_setup
    event_detection_setup/main.cpp
    )
add_executable(event_classification_setup
    event_classification_setup/main.cpp
    event_classification_setup/CrossValidationSuccess.h event_classification_setup/SerializeEventLabelManager.h)

add_executable(event_classification_setup_analzer
    event_classification_setup/results_printer.cpp
    event_classification_setup/CrossValidationSuccess.h
    event_classification_setup/SerializeEventLabelManager.h
    )

add_executable(slimmed_validation
    event_classification_setup/slimmed_validation.cpp
    event_classification_setup/CrossValidationSuccess.h
    event_classification_setup/SerializeEventLabelManager.h
    )
add_executable(integrated_speed_setup
    integrated_speed_setup/main.cpp
    )
target_link_libraries(simple_setup ${experiment_deps})
target_link_libraries(event_detection_setup ${experiment_deps})
target_link_libraries(event_classification_setup ${experiment_deps})
target_link_libraries(event_classification_setup_analzer ${experiment_deps})
target_link_libraries(slimmed_validation ${experiment_deps})
target_link_libraries(integrated_speed_setup ${experiment_deps})
