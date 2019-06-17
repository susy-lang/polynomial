if (OSSFUZZ)
    add_subdirectory(ossfuzz)
endif()

add_subdirectory(yulInterpreter)
add_executable(yulrun yulrun.cpp)
target_link_libraries(yulrun PRIVATE yulInterpreter libpolc svmasm ${Boost_PROGRAM_OPTIONS_LIBRARIES})

add_executable(polfuzzer afl_fuzzer.cpp fuzzer_common.cpp)
target_link_libraries(polfuzzer PRIVATE libpolc svmasm ${Boost_PROGRAM_OPTIONS_LIBRARIES} ${Boost_SYSTEM_LIBRARIES})

add_executable(yulopti yulopti.cpp)
target_link_libraries(yulopti PRIVATE polynomial ${Boost_PROGRAM_OPTIONS_LIBRARIES} ${Boost_SYSTEM_LIBRARIES})

add_executable(ipoltest
	ipoltest.cpp
	IpolTestOptions.cpp
	../Options.cpp
	../Common.cpp
	../TestCase.cpp
	../libpolynomial/util/TestFileParser.cpp
	../libpolynomial/util/TestFunctionCall.cpp
	../libpolynomial/SyntaxTest.cpp
	../libpolynomial/SemanticTest.cpp
	../libpolynomial/AnalysisFramework.cpp
	../libpolynomial/PolynomialExecutionFramework.cpp
	../ExecutionFramework.cpp
	../RPCSession.cpp
	../libpolynomial/ASTJSONTest.cpp
	../libpolynomial/SMTCheckerJSONTest.cpp
	../libyul/ObjectCompilerTest.cpp
	../libyul/YulOptimizerTest.cpp
	../libyul/YulInterpreterTest.cpp
)
target_link_libraries(ipoltest PRIVATE libpolc polynomial yulInterpreter svmasm ${Boost_PROGRAM_OPTIONS_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})
