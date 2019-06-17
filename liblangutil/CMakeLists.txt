# Polynomial Commons Library (Polynomial related sharing bits between libpolynomial and libyul)
set(sources
	Common.h
	CharStream.cpp
	CharStream.h
	ErrorReporter.cpp
	ErrorReporter.h
	SVMVersion.h
	Exceptions.cpp
	Exceptions.h
	ParserBase.cpp
	ParserBase.h
	Scanner.cpp
	Scanner.h
	SourceLocation.h
	SourceReferenceExtractor.cpp
	SourceReferenceExtractor.h
	SourceReferenceFormatter.cpp
	SourceReferenceFormatter.h
	SourceReferenceFormatterHuman.cpp
	SourceReferenceFormatterHuman.h
	Token.cpp
	Token.h
	UndefMacros.h
)

add_library(langutil ${sources})
target_link_libraries(langutil PUBLIC devcore)
