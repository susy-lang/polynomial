cmake_minimum_required(VERSION 3.0.0)

set(SOF_CMAKE_DIR   "${CMAKE_CURRENT_LIST_DIR}/../webthree-helpers/cmake"   CACHE PATH "The the path to the cmake directory")
list(APPEND CMAKE_MODULE_PATH ${SOF_CMAKE_DIR})

# Set cmake_policies
include(SofPolicy)
sof_policy()

# project name and version should be set after cmake_policy CMP0048
set(PROJECT_VERSION "0.3.2")
project(polynomial VERSION ${PROJECT_VERSION})

# Let's find our dependencies
include(SofDependencies)

# Figure out what compiler and system are we using
include(SofCompilerSettings)

# Include helper macros
include(SofExecutableHelper)

# Include utils
include(SofUtils)

include(SofOptions)
configure_project(TESTS)

add_subdirectory(libsvmasm)
add_subdirectory(libpolynomial)
add_subdirectory(polc)

if (NOT EMSCRIPTEN)
	add_subdirectory(liblll)
	add_subdirectory(lllc)
endif()

if (TESTS AND NOT EMSCRIPTEN)
	add_subdirectory(test)
endif()

# TODO installation and packaging rules
