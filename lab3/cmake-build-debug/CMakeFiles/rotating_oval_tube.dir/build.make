# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.15

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

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\JetBrains\CLion 2019.3.5\bin\cmake\win\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\JetBrains\CLion 2019.3.5\bin\cmake\win\bin\cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\CLionProjects\rotating_oval_tube

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\CLionProjects\rotating_oval_tube\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/rotating_oval_tube.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/rotating_oval_tube.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/rotating_oval_tube.dir/flags.make

CMakeFiles/rotating_oval_tube.dir/main.cpp.obj: CMakeFiles/rotating_oval_tube.dir/flags.make
CMakeFiles/rotating_oval_tube.dir/main.cpp.obj: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\CLionProjects\rotating_oval_tube\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/rotating_oval_tube.dir/main.cpp.obj"
	C:\MinGW\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\rotating_oval_tube.dir\main.cpp.obj -c C:\CLionProjects\rotating_oval_tube\main.cpp

CMakeFiles/rotating_oval_tube.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rotating_oval_tube.dir/main.cpp.i"
	C:\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\CLionProjects\rotating_oval_tube\main.cpp > CMakeFiles\rotating_oval_tube.dir\main.cpp.i

CMakeFiles/rotating_oval_tube.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rotating_oval_tube.dir/main.cpp.s"
	C:\MinGW\bin\g++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\CLionProjects\rotating_oval_tube\main.cpp -o CMakeFiles\rotating_oval_tube.dir\main.cpp.s

# Object files for target rotating_oval_tube
rotating_oval_tube_OBJECTS = \
"CMakeFiles/rotating_oval_tube.dir/main.cpp.obj"

# External object files for target rotating_oval_tube
rotating_oval_tube_EXTERNAL_OBJECTS =

rotating_oval_tube.exe: CMakeFiles/rotating_oval_tube.dir/main.cpp.obj
rotating_oval_tube.exe: CMakeFiles/rotating_oval_tube.dir/build.make
rotating_oval_tube.exe: CMakeFiles/rotating_oval_tube.dir/linklibs.rsp
rotating_oval_tube.exe: CMakeFiles/rotating_oval_tube.dir/objects1.rsp
rotating_oval_tube.exe: CMakeFiles/rotating_oval_tube.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\CLionProjects\rotating_oval_tube\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable rotating_oval_tube.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\rotating_oval_tube.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/rotating_oval_tube.dir/build: rotating_oval_tube.exe

.PHONY : CMakeFiles/rotating_oval_tube.dir/build

CMakeFiles/rotating_oval_tube.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\rotating_oval_tube.dir\cmake_clean.cmake
.PHONY : CMakeFiles/rotating_oval_tube.dir/clean

CMakeFiles/rotating_oval_tube.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\CLionProjects\rotating_oval_tube C:\CLionProjects\rotating_oval_tube C:\CLionProjects\rotating_oval_tube\cmake-build-debug C:\CLionProjects\rotating_oval_tube\cmake-build-debug C:\CLionProjects\rotating_oval_tube\cmake-build-debug\CMakeFiles\rotating_oval_tube.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/rotating_oval_tube.dir/depend

