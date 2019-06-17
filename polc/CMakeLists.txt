cmake_policy(SET CMP0015 NEW)
set(CMAKE_AUTOMOC OFF)

aux_source_directory(. SRC_LIST)
list(REMOVE_ITEM SRC_LIST "./jsonCompiler.cpp")

include_directories(BEFORE ${JSONCPP_INCLUDE_DIRS})
include_directories(BEFORE ..)
include_directories(${Boost_INCLUDE_DIRS})

set(EXECUTABLE polc)

file(GLOB HEADERS "*.h")
add_executable(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

add_dependencies(${EXECUTABLE} BuildInfo.h)

target_link_libraries(${EXECUTABLE} ${Boost_FILESYSTEM_LIBRARIES})
target_link_libraries(${EXECUTABLE} ${Boost_PROGRAM_OPTIONS_LIBRARIES})
target_link_libraries(${EXECUTABLE} polynomial)

if (APPLE)
	install(TARGETS ${EXECUTABLE} DESTINATION bin)
else()
	sof_install_executable(${EXECUTABLE})
endif()

add_library(poljson jsonCompiler.cpp ${HEADERS})
target_link_libraries(poljson polynomial)
