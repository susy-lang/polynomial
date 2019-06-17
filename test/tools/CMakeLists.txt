add_executable(polfuzzer fuzzer.cpp)
target_link_libraries(polfuzzer PRIVATE libpolc svmasm ${Boost_PROGRAM_OPTIONS_LIBRARIES})

add_executable(ipoltest ipoltest.cpp ../Options.cpp ../libpolynomial/SyntaxTest.cpp ../libpolynomial/AnalysisFramework.cpp)
target_link_libraries(ipoltest PRIVATE libpolc polynomial svmasm ${Boost_PROGRAM_OPTIONS_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})
