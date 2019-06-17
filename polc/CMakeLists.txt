set(
	sources
	CommandLineInterface.cpp CommandLineInterface.h
	main.cpp
)

add_executable(polc ${sources})
target_link_libraries(polc PRIVATE polynomial ${Boost_PROGRAM_OPTIONS_LIBRARIES})

include(GNUInstallDirs)
install(TARGETS polc DESTINATION "${CMAKE_INSTALL_BINDIR}")

if (EMSCRIPTEN)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORTED_FUNCTIONS='[\"_compileJSON\",\"_license\",\"_version\",\"_compileJSONMulti\",\"_compileJSONCallback\",\"_compileStandard\"]' -s RESERVED_FUNCTION_POINTERS=20")
	add_executable(poljson jsonCompiler.cpp)
else()
	add_library(poljson jsonCompiler.cpp)
endif()
target_link_libraries(poljson PRIVATE polynomial)

if(POLC_LINK_STATIC AND UNIX AND NOT APPLE)
	# Produce polc as statically linked binary (includes C/C++ standard libraries)
	# This is not supported on macOS, see
	# https://developer.apple.com/library/content/qa/qa1118/_index.html.
	set_target_properties(
		polc PROPERTIES
		LINK_FLAGS -static
		LINK_SEARCH_START_STATIC ON
		LINK_SEARCH_END_STATIC ON
	)
endif()