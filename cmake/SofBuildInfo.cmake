function(create_build_info NAME)

	# Set build platform; to be written to BuildInfo.h
	set(SOF_BUILD_OS "${CMAKE_SYSTEM_NAME}")

	if (CMAKE_COMPILER_IS_MINGW)
		set(SOF_BUILD_COMPILER "mingw")
	elseif (CMAKE_COMPILER_IS_MSYS)
		set(SOF_BUILD_COMPILER "msys")
	elseif (CMAKE_COMPILER_IS_GNUCXX)
		set(SOF_BUILD_COMPILER "g++")
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		set(SOF_BUILD_COMPILER "msvc")
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(SOF_BUILD_COMPILER "clang")
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
		set(SOF_BUILD_COMPILER "appleclang")
	else ()
		set(SOF_BUILD_COMPILER "unknown")
	endif ()

	set(SOF_BUILD_PLATFORM "${SOF_BUILD_OS}/${SOF_BUILD_COMPILER}")

	#cmake build type may be not speCified when using msvc
	if (CMAKE_BUILD_TYPE)
		set(_cmake_build_type ${CMAKE_BUILD_TYPE})
	else()
		set(_cmake_build_type "${CMAKE_CFG_INTDIR}")
	endif()

	# Generate header file containing useful build information
	add_custom_target(${NAME}_BuildInfo.h ALL
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		COMMAND ${CMAKE_COMMAND} -DSOF_SOURCE_DIR="${PROJECT_SOURCE_DIR}" -DSOF_BUILDINFO_IN="${SOF_CMAKE_DIR}/templates/BuildInfo.h.in" -DSOF_DST_DIR="${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}" -DSOF_CMAKE_DIR="${SOF_CMAKE_DIR}"
		-DSOF_BUILD_TYPE="${_cmake_build_type}"
		-DSOF_BUILD_OS="${SOF_BUILD_OS}"
		-DSOF_BUILD_COMPILER="${SOF_BUILD_COMPILER}"
		-DSOF_BUILD_PLATFORM="${SOF_BUILD_PLATFORM}"
		-DSOF_BUILD_NUMBER="${BUILD_NUMBER}"
		-DSOF_VERSION_SUFFIX="${VERSION_SUFFIX}"
		-DPROJECT_VERSION="${PROJECT_VERSION}"
		-P "${SOF_SCRIPTS_DIR}/buildinfo.cmake"
		)
	include_directories(BEFORE ${PROJECT_BINARY_DIR})
endfunction()
