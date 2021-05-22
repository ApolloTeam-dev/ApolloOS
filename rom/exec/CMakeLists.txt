cmake_minimum_required(VERSION 3.17)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(WARNING [[In-source build detected, please eg. create a new directory and use `cmake ..`]])
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

option(DEBUG "Enable debug mode in engine" OFF)
option(APOLLO "Enable AC68080 specifics" ON)
option(DEBUG_RELEASE "Enable options for debug release" OFF)

set(PROJECT_NAME "exec")

if(DEBUG_RELEASE)
	set(CMAKE_BUILD_TYPE "Debug")
	set(DEBUG ON)
endif()


# Get the version from the config file
FILE(READ "${PROJECT_NAME}.conf" contents)

# Convert file contents into a CMake list (where each element in the list
# is one line of the file)
#
STRING(REGEX REPLACE ";" "\\\\;" contents "${contents}")
STRING(REGEX REPLACE "\n" ";" contents "${contents}")

set(VERSION_REGEX "version\ ([0-9]+).([0-9])")

foreach(item ${contents})
	if(item MATCHES ${VERSION_REGEX})
		STRING(REGEX REPLACE
				${VERSION_REGEX}
				"\\1.\\2" PROJECT_VERSION ${item})
	endif()
endforeach()

# Project specifics
project(${PROJECT_NAME}
		VERSION ${PROJECT_VERSION}
		LANGUAGES C ASM)

# Set to true/ON if it's a library
set(OS_LIBRARY ON)

set(PROJECT_BINARY_FILENAME "${PROJECT_NAME}")
if(OS_LIBRARY)
	list(APPEND PROJECT_BINARY_FILENAME ".library")
	list(JOIN PROJECT_BINARY_FILENAME "" PROJECT_BINARY_FILENAME)
endif()

# Make version variables
string(REPLACE "." ";" VERSION_LIST ${PROJECT_VERSION})
list(GET VERSION_LIST 0 PROJECT_MAJOR_VERSION)
list(GET VERSION_LIST 1 PROJECT_MINOR_VERSION)

# Print out which project we're compiling
message(STATUS "${PROJECT_BINARY_FILENAME} version: ${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}")

# Lowercase project name for binaries and packaging
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)
string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_LOWER)
string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} SYSTEM_ARCH_LOWER)

include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake")

# Lets include the files to compile
file(GLOB PROJECT_SOURCE_FILES "*.c" "*.h" "*.asm")

set(INIT_FILES "exec_init.c;prepareexecbase.c")


if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	# We're running standalone guys!


	# Fetch AROS support include files to the cmake build directory
	file(GLOB AROS_SUPPORT_INCLUDES "../../compiler/arossupport/include/*.h")

	foreach(support_include ${AROS_SUPPORT_INCLUDES})
		get_filename_component(support_include_filename ${support_include} NAME)
		configure_file(${support_include} "${PROJECT_BINARY_DIR}/CMakeFiles/include/aros/${support_include_filename}" COPYONLY)
	endforeach()

	# Fetch AROS m68k support include files to the cmake build directory
	file(GLOB AROS_M68K_SUPPORT_INCLUDES "../../arch/m68k-all/include/aros/*.h")

	foreach(m68k_include ${AROS_M68K_SUPPORT_INCLUDES})
		get_filename_component(m68k_include_filename ${m68k_include} NAME)
		configure_file(${m68k_include} "${PROJECT_BINARY_DIR}/CMakeFiles/include/aros/m68k/${m68k_include_filename}" COPYONLY)
	endforeach()

	include_directories("../../compiler/include" "../../arch/m68k-all/include/" "../../arch/m68k-amiga/include/")
else()
	# We're building as part of the full ApolloOS experience! We don't need to do anything
endif()


if(OS_LIBRARY)
	add_library(${PROJECT_BINARY_FILENAME} ${INIT_FILES} ${PROJECT_SOURCE_FILES})
else()
	add_executable(${PROJECT_BINARY_FILENAME} ${PROJECT_SOURCE_FILES})
endif()


# We're compiling for Amiga computers, so add the proper define for this
add_definitions("-DAROS_ARCH_amiga")


include_directories(${PROJECT_SOURCE_DIR} "${PROJECT_BINARY_DIR}/CMakeFiles/include")


#message(${PROJECT_SOURCE_FILES})

