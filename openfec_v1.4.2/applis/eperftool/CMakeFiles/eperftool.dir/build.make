# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2

# Include any dependencies generated for this target.
include applis/eperftool/CMakeFiles/eperftool.dir/depend.make

# Include the progress variables for this target.
include applis/eperftool/CMakeFiles/eperftool.dir/progress.make

# Include the compile flags for this target's objects.
include applis/eperftool/CMakeFiles/eperftool.dir/flags.make

applis/eperftool/CMakeFiles/eperftool.dir/blocking_struct.c.o: applis/eperftool/CMakeFiles/eperftool.dir/flags.make
applis/eperftool/CMakeFiles/eperftool.dir/blocking_struct.c.o: applis/eperftool/blocking_struct.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object applis/eperftool/CMakeFiles/eperftool.dir/blocking_struct.c.o"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/eperftool.dir/blocking_struct.c.o   -c /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/blocking_struct.c

applis/eperftool/CMakeFiles/eperftool.dir/blocking_struct.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/eperftool.dir/blocking_struct.c.i"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/blocking_struct.c > CMakeFiles/eperftool.dir/blocking_struct.c.i

applis/eperftool/CMakeFiles/eperftool.dir/blocking_struct.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/eperftool.dir/blocking_struct.c.s"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/blocking_struct.c -o CMakeFiles/eperftool.dir/blocking_struct.c.s

applis/eperftool/CMakeFiles/eperftool.dir/callbacks.c.o: applis/eperftool/CMakeFiles/eperftool.dir/flags.make
applis/eperftool/CMakeFiles/eperftool.dir/callbacks.c.o: applis/eperftool/callbacks.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object applis/eperftool/CMakeFiles/eperftool.dir/callbacks.c.o"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/eperftool.dir/callbacks.c.o   -c /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/callbacks.c

applis/eperftool/CMakeFiles/eperftool.dir/callbacks.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/eperftool.dir/callbacks.c.i"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/callbacks.c > CMakeFiles/eperftool.dir/callbacks.c.i

applis/eperftool/CMakeFiles/eperftool.dir/callbacks.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/eperftool.dir/callbacks.c.s"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/callbacks.c -o CMakeFiles/eperftool.dir/callbacks.c.s

applis/eperftool/CMakeFiles/eperftool.dir/codec_instance_mgmt.c.o: applis/eperftool/CMakeFiles/eperftool.dir/flags.make
applis/eperftool/CMakeFiles/eperftool.dir/codec_instance_mgmt.c.o: applis/eperftool/codec_instance_mgmt.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object applis/eperftool/CMakeFiles/eperftool.dir/codec_instance_mgmt.c.o"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/eperftool.dir/codec_instance_mgmt.c.o   -c /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/codec_instance_mgmt.c

applis/eperftool/CMakeFiles/eperftool.dir/codec_instance_mgmt.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/eperftool.dir/codec_instance_mgmt.c.i"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/codec_instance_mgmt.c > CMakeFiles/eperftool.dir/codec_instance_mgmt.c.i

applis/eperftool/CMakeFiles/eperftool.dir/codec_instance_mgmt.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/eperftool.dir/codec_instance_mgmt.c.s"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/codec_instance_mgmt.c -o CMakeFiles/eperftool.dir/codec_instance_mgmt.c.s

applis/eperftool/CMakeFiles/eperftool.dir/command_line.c.o: applis/eperftool/CMakeFiles/eperftool.dir/flags.make
applis/eperftool/CMakeFiles/eperftool.dir/command_line.c.o: applis/eperftool/command_line.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object applis/eperftool/CMakeFiles/eperftool.dir/command_line.c.o"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/eperftool.dir/command_line.c.o   -c /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/command_line.c

applis/eperftool/CMakeFiles/eperftool.dir/command_line.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/eperftool.dir/command_line.c.i"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/command_line.c > CMakeFiles/eperftool.dir/command_line.c.i

applis/eperftool/CMakeFiles/eperftool.dir/command_line.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/eperftool.dir/command_line.c.s"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/command_line.c -o CMakeFiles/eperftool.dir/command_line.c.s

applis/eperftool/CMakeFiles/eperftool.dir/eperftool.c.o: applis/eperftool/CMakeFiles/eperftool.dir/flags.make
applis/eperftool/CMakeFiles/eperftool.dir/eperftool.c.o: applis/eperftool/eperftool.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object applis/eperftool/CMakeFiles/eperftool.dir/eperftool.c.o"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/eperftool.dir/eperftool.c.o   -c /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/eperftool.c

applis/eperftool/CMakeFiles/eperftool.dir/eperftool.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/eperftool.dir/eperftool.c.i"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/eperftool.c > CMakeFiles/eperftool.dir/eperftool.c.i

applis/eperftool/CMakeFiles/eperftool.dir/eperftool.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/eperftool.dir/eperftool.c.s"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/eperftool.c -o CMakeFiles/eperftool.dir/eperftool.c.s

applis/eperftool/CMakeFiles/eperftool.dir/globals.c.o: applis/eperftool/CMakeFiles/eperftool.dir/flags.make
applis/eperftool/CMakeFiles/eperftool.dir/globals.c.o: applis/eperftool/globals.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building C object applis/eperftool/CMakeFiles/eperftool.dir/globals.c.o"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/eperftool.dir/globals.c.o   -c /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/globals.c

applis/eperftool/CMakeFiles/eperftool.dir/globals.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/eperftool.dir/globals.c.i"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/globals.c > CMakeFiles/eperftool.dir/globals.c.i

applis/eperftool/CMakeFiles/eperftool.dir/globals.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/eperftool.dir/globals.c.s"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/globals.c -o CMakeFiles/eperftool.dir/globals.c.s

applis/eperftool/CMakeFiles/eperftool.dir/receiver.c.o: applis/eperftool/CMakeFiles/eperftool.dir/flags.make
applis/eperftool/CMakeFiles/eperftool.dir/receiver.c.o: applis/eperftool/receiver.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building C object applis/eperftool/CMakeFiles/eperftool.dir/receiver.c.o"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/eperftool.dir/receiver.c.o   -c /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/receiver.c

applis/eperftool/CMakeFiles/eperftool.dir/receiver.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/eperftool.dir/receiver.c.i"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/receiver.c > CMakeFiles/eperftool.dir/receiver.c.i

applis/eperftool/CMakeFiles/eperftool.dir/receiver.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/eperftool.dir/receiver.c.s"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/receiver.c -o CMakeFiles/eperftool.dir/receiver.c.s

applis/eperftool/CMakeFiles/eperftool.dir/sender.c.o: applis/eperftool/CMakeFiles/eperftool.dir/flags.make
applis/eperftool/CMakeFiles/eperftool.dir/sender.c.o: applis/eperftool/sender.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building C object applis/eperftool/CMakeFiles/eperftool.dir/sender.c.o"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/eperftool.dir/sender.c.o   -c /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/sender.c

applis/eperftool/CMakeFiles/eperftool.dir/sender.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/eperftool.dir/sender.c.i"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/sender.c > CMakeFiles/eperftool.dir/sender.c.i

applis/eperftool/CMakeFiles/eperftool.dir/sender.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/eperftool.dir/sender.c.s"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/sender.c -o CMakeFiles/eperftool.dir/sender.c.s

applis/eperftool/CMakeFiles/eperftool.dir/tx_simulator.c.o: applis/eperftool/CMakeFiles/eperftool.dir/flags.make
applis/eperftool/CMakeFiles/eperftool.dir/tx_simulator.c.o: applis/eperftool/tx_simulator.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building C object applis/eperftool/CMakeFiles/eperftool.dir/tx_simulator.c.o"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/eperftool.dir/tx_simulator.c.o   -c /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/tx_simulator.c

applis/eperftool/CMakeFiles/eperftool.dir/tx_simulator.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/eperftool.dir/tx_simulator.c.i"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/tx_simulator.c > CMakeFiles/eperftool.dir/tx_simulator.c.i

applis/eperftool/CMakeFiles/eperftool.dir/tx_simulator.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/eperftool.dir/tx_simulator.c.s"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/tx_simulator.c -o CMakeFiles/eperftool.dir/tx_simulator.c.s

# Object files for target eperftool
eperftool_OBJECTS = \
"CMakeFiles/eperftool.dir/blocking_struct.c.o" \
"CMakeFiles/eperftool.dir/callbacks.c.o" \
"CMakeFiles/eperftool.dir/codec_instance_mgmt.c.o" \
"CMakeFiles/eperftool.dir/command_line.c.o" \
"CMakeFiles/eperftool.dir/eperftool.c.o" \
"CMakeFiles/eperftool.dir/globals.c.o" \
"CMakeFiles/eperftool.dir/receiver.c.o" \
"CMakeFiles/eperftool.dir/sender.c.o" \
"CMakeFiles/eperftool.dir/tx_simulator.c.o"

# External object files for target eperftool
eperftool_EXTERNAL_OBJECTS =

bin/Release/eperftool: applis/eperftool/CMakeFiles/eperftool.dir/blocking_struct.c.o
bin/Release/eperftool: applis/eperftool/CMakeFiles/eperftool.dir/callbacks.c.o
bin/Release/eperftool: applis/eperftool/CMakeFiles/eperftool.dir/codec_instance_mgmt.c.o
bin/Release/eperftool: applis/eperftool/CMakeFiles/eperftool.dir/command_line.c.o
bin/Release/eperftool: applis/eperftool/CMakeFiles/eperftool.dir/eperftool.c.o
bin/Release/eperftool: applis/eperftool/CMakeFiles/eperftool.dir/globals.c.o
bin/Release/eperftool: applis/eperftool/CMakeFiles/eperftool.dir/receiver.c.o
bin/Release/eperftool: applis/eperftool/CMakeFiles/eperftool.dir/sender.c.o
bin/Release/eperftool: applis/eperftool/CMakeFiles/eperftool.dir/tx_simulator.c.o
bin/Release/eperftool: applis/eperftool/CMakeFiles/eperftool.dir/build.make
bin/Release/eperftool: bin/Release/libopenfec.a
bin/Release/eperftool: applis/eperftool/CMakeFiles/eperftool.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Linking C executable ../../bin/Release/eperftool"
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/eperftool.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
applis/eperftool/CMakeFiles/eperftool.dir/build: bin/Release/eperftool

.PHONY : applis/eperftool/CMakeFiles/eperftool.dir/build

applis/eperftool/CMakeFiles/eperftool.dir/clean:
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool && $(CMAKE_COMMAND) -P CMakeFiles/eperftool.dir/cmake_clean.cmake
.PHONY : applis/eperftool/CMakeFiles/eperftool.dir/clean

applis/eperftool/CMakeFiles/eperftool.dir/depend:
	cd /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2 /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2 /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool /home/gxh/works/huichang/svc_codec/bizconf_svc_codec/openfec_v1.4.2/applis/eperftool/CMakeFiles/eperftool.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : applis/eperftool/CMakeFiles/eperftool.dir/depend

