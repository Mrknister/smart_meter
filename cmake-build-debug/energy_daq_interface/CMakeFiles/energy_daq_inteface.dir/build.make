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
include energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/depend.make

# Include the progress variables for this target.
include energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/progress.make

# Include the compile flags for this target's objects.
include energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/flags.make

energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.o: energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/flags.make
energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.o: ../energy_daq_interface/src/daq_interface.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.o"
	cd /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/energy_daq_interface && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.o -c /home/jan/Dropbox/Projects/C++/smart_screen/energy_daq_interface/src/daq_interface.cpp

energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.i"
	cd /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/energy_daq_interface && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/jan/Dropbox/Projects/C++/smart_screen/energy_daq_interface/src/daq_interface.cpp > CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.i

energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.s"
	cd /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/energy_daq_interface && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/jan/Dropbox/Projects/C++/smart_screen/energy_daq_interface/src/daq_interface.cpp -o CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.s

energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.o.requires:

.PHONY : energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.o.requires

energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.o.provides: energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.o.requires
	$(MAKE) -f energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/build.make energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.o.provides.build
.PHONY : energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.o.provides

energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.o.provides.build: energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.o


# Object files for target energy_daq_inteface
energy_daq_inteface_OBJECTS = \
"CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.o"

# External object files for target energy_daq_inteface
energy_daq_inteface_EXTERNAL_OBJECTS =

energy_daq_interface/libenergy_daq_inteface.a: energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.o
energy_daq_interface/libenergy_daq_inteface.a: energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/build.make
energy_daq_interface/libenergy_daq_inteface.a: energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libenergy_daq_inteface.a"
	cd /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/energy_daq_interface && $(CMAKE_COMMAND) -P CMakeFiles/energy_daq_inteface.dir/cmake_clean_target.cmake
	cd /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/energy_daq_interface && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/energy_daq_inteface.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/build: energy_daq_interface/libenergy_daq_inteface.a

.PHONY : energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/build

energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/requires: energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/src/daq_interface.cpp.o.requires

.PHONY : energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/requires

energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/clean:
	cd /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/energy_daq_interface && $(CMAKE_COMMAND) -P CMakeFiles/energy_daq_inteface.dir/cmake_clean.cmake
.PHONY : energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/clean

energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/depend:
	cd /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jan/Dropbox/Projects/C++/smart_screen /home/jan/Dropbox/Projects/C++/smart_screen/energy_daq_interface /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/energy_daq_interface /home/jan/Dropbox/Projects/C++/smart_screen/cmake-build-debug/energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : energy_daq_interface/CMakeFiles/energy_daq_inteface.dir/depend
