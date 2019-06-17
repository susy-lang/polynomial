if (EMSCRIPTEN)
	# Specify which functions to export in poljson.js.
	# Note that additional Emscripten-generated methods needed by polc-js are
	# defined to be exported in cmake/SofCompilerSettings.cmake.
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORTED_FUNCTIONS='[\"_polynomial_license\",\"_polynomial_version\",\"_polynomial_compile\"]' -s RESERVED_FUNCTION_POINTERS=20")
	add_executable(poljson libpolc.cpp libpolc.h)
	target_link_libraries(poljson PRIVATE polynomial)
else()
	add_library(libpolc libpolc.cpp libpolc.h)
	set_target_properties(libpolc PROPERTIES OUTPUT_NAME polc)
	target_link_libraries(libpolc PRIVATE polynomial)
endif()
