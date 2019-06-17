set(sources
	SVMInstructionInterpreter.h
	SVMInstructionInterpreter.cpp
	Interpreter.h
	Interpreter.cpp
)

add_library(yulInterpreter ${sources})
target_link_libraries(yulInterpreter PUBLIC yul polynomial devcore)
