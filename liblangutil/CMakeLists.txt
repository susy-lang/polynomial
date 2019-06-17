# Polynomial Commons Library (Polynomial related sharing bits between libpolynomial and libyul)
set(sources
	CharStream.cpp
	ErrorReporter.cpp
	Exceptions.cpp
	ParserBase.cpp
	Scanner.cpp
	SourceReferenceFormatter.cpp
	Token.cpp
)

add_library(langutil ${sources})
target_link_libraries(langutil PUBLIC devcore)
