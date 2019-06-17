cmake_minimum_required(VERSION 2.8.12)

cmake_policy(SET CMP0015 NEW)
# let cmake autolink dependencies on windows
cmake_policy(SET CMP0020 NEW)
# this policy was introduced in cmake 3.0
# remove if, once 3.0 will be used on unix
if (${CMAKE_MAJOR_VERSION} GREATER 2)
	cmake_policy(SET CMP0043 OLD)
endif()

# TODO use version from Version.h
set(PROJECT_VERSION "0.9.42")
if (${CMAKE_VERSION} VERSION_GREATER 3.0)
	cmake_policy(SET CMP0042 OLD) 	# fix MACOSX_RPATH
	cmake_policy(SET CMP0048 NEW) 	# allow VERSION argument in project()
	project(polynomial VERSION ${PROJECT_VERSION})
else()
	project(polynomial)
endif()

# Figure out environment.
set(CPPSOPHON_BUILD "${CMAKE_CURRENT_SOURCE_DIR}/../cpp-sophon/${BUILD_DIR_NAME}" CACHE STRING "The absolute build directory of cpp-sophon.")
set(SOF_CMAKE_DIR   "${CMAKE_CURRENT_LIST_DIR}/../cpp-sophon-cmake"   CACHE PATH "The the path to the cmake directory")
set(SOF_DIR         "${CMAKE_CURRENT_LIST_DIR}/../cpp-sophon"         CACHE PATH "The path to the cpp-sophon directory")
set(BUILD_DIR_NAME  "build"                                            CACHE STRING "The name of the build directory in cpp-sophon")
set(SOF_BUILD_DIR   "${SOF_DIR}/${BUILD_DIR_NAME}")

list(APPEND CMAKE_MODULE_PATH ${SOF_CMAKE_DIR})

include(SofDependencies)
include(SofExecutableHelper)
include(SofCompilerSettings)

include_directories(BEFORE ${JSONCPP_INCLUDE_DIRS})
include_directories(BEFORE ..)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(libpolynomial)
# Include a directory with BuildInfo.h
include_directories(${SOF_BUILD_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../cpp-sophon)
include_directories(${CPPSOPHON_BUILD})

file(GLOB_RECURSE SRC_LIST "libpolynomial/*.cpp")
file(GLOB_RECURSE HEADERS "libpolynomial/*.h")

set(EXECUTABLE polynomial)
add_library(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

# TODO installation and packaging rules
