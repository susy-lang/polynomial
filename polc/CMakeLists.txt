aux_source_directory(. SRC_LIST)
list(REMOVE_ITEM SRC_LIST "./jsonCompiler.cpp")

include_directories(BEFORE ..)

set(EXECUTABLE polc)

file(GLOB HEADERS "*.h")
sof_simple_add_executable(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

sof_use(${EXECUTABLE} REQUIRED Polynomial::polynomial)
target_link_libraries(${EXECUTABLE} ${Boost_PROGRAM_OPTIONS_LIBRARIES})

if (APPLE)
	install(TARGETS ${EXECUTABLE} DESTINATION bin)
else()
	sof_install_executable(${EXECUTABLE})
endif()

if (EMSCRIPTEN)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORTED_FUNCTIONS='[\"_compileJSON\",\"_version\",\"_compileJSONMulti\",\"_compileJSONCallback\",\"_compileStandard\"]' -s RESERVED_FUNCTION_POINTERS=20")
	add_executable(poljson jsonCompiler.cpp ${HEADERS})
	sof_use(poljson REQUIRED Polynomial::polynomial)
else()
	add_library(poljson jsonCompiler.cpp ${HEADERS})
	target_link_libraries(poljson polynomial)
endif()
