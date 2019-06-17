add_executable(polfuzzer fuzzer.cpp)
target_link_libraries(polfuzzer PRIVATE libpolc svmasm ${Boost_PROGRAM_OPTIONS_LIBRARIES} ${Boost_SYSTEM_LIBRARIES})

add_executable(yulopti yulopti.cpp)
target_link_libraries(yulopti PRIVATE polynomial ${Boost_PROGRAM_OPTIONS_LIBRARIES} ${Boost_SYSTEM_LIBRARIES})

add_executable(ipoltest
	ipoltest.cpp
	../Options.cpp
	../Common.cpp
	../TestCase.cpp
	../libpolynomial/SyntaxTest.cpp
	../libpolynomial/AnalysisFramework.cpp
	../libpolynomial/PolynomialExecutionFramework.cpp
	../ExecutionFramework.cpp
	../RPCSession.cpp
	../libpolynomial/ASTJSONTest.cpp
	../libpolynomial/SMTCheckerJSONTest.cpp
	../libyul/ObjectCompilerTest.cpp
	../libyul/YulOptimizerTest.cpp
)
target_link_libraries(ipoltest PRIVATE libpolc polynomial svmasm ${Boost_PROGRAM_OPTIONS_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES})
