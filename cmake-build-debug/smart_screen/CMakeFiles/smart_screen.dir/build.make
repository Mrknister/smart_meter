# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.6

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/clion/bin/cmake/bin/cmake

# The command to remove a file.
RM = /opt/clion/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/jan/Dropbox/Projects/C++/smart_screen

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug

# Include any dependencies generated for this target.
include smart_screen/CMakeFiles/smart_screen.dir/depend.make

# Include the progress variables for this target.
include smart_screen/CMakeFiles/smart_screen.dir/progress.make

# Include the compile flags for this target's objects.
include smart_screen/CMakeFiles/smart_screen.dir/flags.make

smart_screen/CMakeFiles/smart_screen.dir/src/main.cpp.o: smart_screen/CMakeFiles/smart_screen.dir/flags.make
smart_screen/CMakeFiles/smart_screen.dir/src/main.cpp.o: ../smart_screen/src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object smart_screen/CMakeFiles/smart_screen.dir/src/main.cpp.o"
	cd /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/smart_screen && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/smart_screen.dir/src/main.cpp.o -c /home/jan/Dropbox/Projects/C++/smart_screen/smart_screen/src/main.cpp

smart_screen/CMakeFiles/smart_screen.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/smart_screen.dir/src/main.cpp.i"
	cd /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/smart_screen && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jan/Dropbox/Projects/C++/smart_screen/smart_screen/src/main.cpp > CMakeFiles/smart_screen.dir/src/main.cpp.i

smart_screen/CMakeFiles/smart_screen.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/smart_screen.dir/src/main.cpp.s"
	cd /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/smart_screen && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jan/Dropbox/Projects/C++/smart_screen/smart_screen/src/main.cpp -o CMakeFiles/smart_screen.dir/src/main.cpp.s

smart_screen/CMakeFiles/smart_screen.dir/src/main.cpp.o.requires:

.PHONY : smart_screen/CMakeFiles/smart_screen.dir/src/main.cpp.o.requires

smart_screen/CMakeFiles/smart_screen.dir/src/main.cpp.o.provides: smart_screen/CMakeFiles/smart_screen.dir/src/main.cpp.o.requires
	$(MAKE) -f smart_screen/CMakeFiles/smart_screen.dir/build.make smart_screen/CMakeFiles/smart_screen.dir/src/main.cpp.o.provides.build
.PHONY : smart_screen/CMakeFiles/smart_screen.dir/src/main.cpp.o.provides

smart_screen/CMakeFiles/smart_screen.dir/src/main.cpp.o.provides.build: smart_screen/CMakeFiles/smart_screen.dir/src/main.cpp.o


# Object files for target smart_screen
smart_screen_OBJECTS = \
"CMakeFiles/smart_screen.dir/src/main.cpp.o"

# External object files for target smart_screen
smart_screen_EXTERNAL_OBJECTS =

smart_screen/smart_screen: smart_screen/CMakeFiles/smart_screen.dir/src/main.cpp.o
smart_screen/smart_screen: smart_screen/CMakeFiles/smart_screen.dir/build.make
smart_screen/smart_screen: dataloader/libdataloader.a
smart_screen/smart_screen: libanalyze/libanalyze.a
smart_screen/smart_screen: event_detector/libevent_detector.a
smart_screen/smart_screen: data_analyzer/libdata_analyzer.a
smart_screen/smart_screen: event_detector/libevent_detector.a
smart_screen/smart_screen: dataloader/libdataloader.a
smart_screen/smart_screen: /usr/lib64/libsndfile.so
smart_screen/smart_screen: /usr/lib64/libboost_serialization.so
smart_screen/smart_screen: /usr/lib64/libhdf5_cpp.so
smart_screen/smart_screen: /usr/lib64/libhdf5.so
smart_screen/smart_screen: /usr/lib64/libz.so
smart_screen/smart_screen: /usr/lib64/libdl.so
smart_screen/smart_screen: /usr/lib64/libm.so
smart_screen/smart_screen: libanalyze/libanalyze.a
smart_screen/smart_screen: libanalyze/libkiss_fft.a
smart_screen/smart_screen: /usr/lib64/libboost_system.so
smart_screen/smart_screen: /usr/lib64/libboost_filesystem.so
smart_screen/smart_screen: /usr/lib64/libboost_date_time.so
smart_screen/smart_screen: ../external/libnabo/build/libnabo.a
smart_screen/smart_screen: smart_screen/CMakeFiles/smart_screen.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable smart_screen"
	cd /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/smart_screen && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/smart_screen.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
smart_screen/CMakeFiles/smart_screen.dir/build: smart_screen/smart_screen

.PHONY : smart_screen/CMakeFiles/smart_screen.dir/build

smart_screen/CMakeFiles/smart_screen.dir/requires: smart_screen/CMakeFiles/smart_screen.dir/src/main.cpp.o.requires

.PHONY : smart_screen/CMakeFiles/smart_screen.dir/requires

smart_screen/CMakeFiles/smart_screen.dir/clean:
	cd /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/smart_screen && $(CMAKE_COMMAND) -P CMakeFiles/smart_screen.dir/cmake_clean.cmake
.PHONY : smart_screen/CMakeFiles/smart_screen.dir/clean

smart_screen/CMakeFiles/smart_screen.dir/depend:
	cd /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jan/Dropbox/Projects/C++/smart_screen /home/jan/Dropbox/Projects/C++/smart_screen/smart_screen /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/smart_screen /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/smart_screen/CMakeFiles/smart_screen.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : smart_screen/CMakeFiles/smart_screen.dir/depend
