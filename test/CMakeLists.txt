file(GLOB_RECURSE sources "*.cpp")
list(REMOVE_ITEM sources "${CMAKE_CURRENT_SOURCE_DIR}/fuzzer.cpp")
file(GLOB_RECURSE headers "*.h")

add_executable(poltest ${sources} ${headers})
target_link_libraries(poltest PRIVATE libpolc polynomial lll svmasm devcore ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})

add_executable(polfuzzer fuzzer.cpp)
target_link_libraries(polfuzzer PRIVATE libpolc svmasm ${Boost_PROGRAM_OPTIONS_LIBRARIES})
