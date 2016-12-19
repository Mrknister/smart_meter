# CMake generated Testfile for 
# Source directory: /home/jan/Dropbox/Projects/C++/smart_screen
# Build directory: /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(ENERGY_TESTS "/home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/test/Test")
subdirs(libanalyze)
subdirs(dataloader)
subdirs(event_detector)
subdirs(data_analyzer)
subdirs(smart_screen)
subdirs(blued_converter)
subdirs(energy_daq_interface)
subdirs(energy_daq)
subdirs(test)
