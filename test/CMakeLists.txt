cmake_policy(SET CMP0015 NEW)

aux_source_directory(. SRC_LIST)
aux_source_directory(libdevcore SRC_LIST)
aux_source_directory(libsvmasm SRC_LIST)
aux_source_directory(libpolynomial SRC_LIST)
aux_source_directory(libjulia SRC_LIST)
aux_source_directory(contracts SRC_LIST)
aux_source_directory(liblll SRC_LIST)
aux_source_directory(libjulia SRC_LIST)

list(REMOVE_ITEM SRC_LIST "./fuzzer.cpp")

get_filename_component(TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)

file(GLOB HEADERS "*.h" "*/*.h")
set(EXECUTABLE poltest)
sof_simple_add_executable(${EXECUTABLE} ${SRC_LIST} ${HEADERS})

sof_use(${EXECUTABLE} REQUIRED Polynomial::polynomial Polynomial::lll)

include_directories(BEFORE ..)
target_link_libraries(${EXECUTABLE} poljson ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})

add_executable(polfuzzer fuzzer.cpp)
target_link_libraries(polfuzzer poljson ${Boost_PROGRAM_OPTIONS_LIBRARIES})
