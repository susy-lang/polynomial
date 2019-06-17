function(sof_apply TARGET REQUIRED SUBMODULE)

	set(POL_DIR "${SOF_CMAKE_DIR}/.." CACHE PATH "The path to the polynomial directory")
	set(POL_BUILD_DIR_NAME  "build" CACHE STRING "The name of the build directory in polynomial repo")
	set(POL_BUILD_DIR "${POL_DIR}/${POL_BUILD_DIR_NAME}")
	set(CMAKE_LIBRARY_PATH ${POL_BUILD_DIR};${CMAKE_LIBRARY_PATH})

	find_package(Polynomial)

	# Hide confusing blank dependency information when using FindPolynomial on itself.
	if (NOT(${MODULE_MAIN} STREQUAL Polynomial))
		sof_show_dependency(POLYNOMIAL polynomial)
	endif()

	target_include_directories(${TARGET} PUBLIC ${Polynomial_INCLUDE_DIRS})

	if (${SUBMODULE} STREQUAL "polsvmasm")
		sof_use(${TARGET} ${REQUIRED} Jsoncpp)
		target_link_libraries(${TARGET} ${Polynomial_POLSVMASM_LIBRARIES})
	endif()

	if (${SUBMODULE} STREQUAL "lll")
		sof_use(${TARGET} ${REQUIRED} Polynomial::polsvmasm)
		target_link_libraries(${TARGET} ${Polynomial_LLL_LIBRARIES})
	endif()

	if (${SUBMODULE} STREQUAL "polynomial" OR ${SUBMODULE} STREQUAL "")
		sof_use(${TARGET} ${REQUIRED} Dev::poldevcore Polynomial::polsvmasm)
		target_link_libraries(${TARGET} ${Polynomial_POLYNOMIAL_LIBRARIES})
	endif()

	target_compile_definitions(${TARGET} PUBLIC SOF_POLYNOMIAL)
endfunction()
