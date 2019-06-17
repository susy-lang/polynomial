cmake_minimum_required(VERSION 3.0.0)

set(SOF_CMAKE_DIR   "${CMAKE_CURRENT_LIST_DIR}/cmake"   CACHE PATH "The the path to the cmake directory")
list(APPEND CMAKE_MODULE_PATH ${SOF_CMAKE_DIR})

# Set cmake_policies
include(SofPolicy)
sof_policy()

# project name and version should be set after cmake_policy CMP0048
set(PROJECT_VERSION "0.4.23")
project(polynomial VERSION ${PROJECT_VERSION})

option(POLC_LINK_STATIC "Link polc executable statically on supported platforms" OFF)
option(LLLC_LINK_STATIC "Link lllc executable statically on supported platforms" OFF)
option(INSTALL_LLLC "Include lllc executable in installation" OFF)

# Setup cccache.
include(SofCcache)

# Let's find our dependencies
include(SofDependencies)
include(jsoncpp)

find_package(Threads)

# Figure out what compiler and system are we using
include(SofCompilerSettings)

# Include utils
include(SofUtils)

# Create license.h from LICENSE.txt and template
# Converting to char array is required due to MSVC's string size limit.
file(READ ${CMAKE_SOURCE_DIR}/LICENSE.txt LICENSE_TEXT HEX)
string(REGEX MATCHALL ".." LICENSE_TEXT "${LICENSE_TEXT}")
string(REGEX REPLACE ";" ",\n\t0x" LICENSE_TEXT "${LICENSE_TEXT}")
set(LICENSE_TEXT "0x${LICENSE_TEXT}")

configure_file("${CMAKE_SOURCE_DIR}/cmake/templates/license.h.in" include/license.h)

include(SofOptions)
configure_project(TESTS)

add_subdirectory(libdevcore)
add_subdirectory(libsvmasm)
add_subdirectory(libpolynomial)
add_subdirectory(libpolc)

if (NOT EMSCRIPTEN)
	add_subdirectory(polc)
	add_subdirectory(liblll)
	add_subdirectory(lllc)
endif()

if (TESTS AND NOT EMSCRIPTEN)
	add_subdirectory(test)
endif()
