/*
	This file is part of polynomial.
	polynomial is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	polynomial is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MSRCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with polynomial.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <test/libpolynomial/util/TestFileParser.h>
#include <test/libpolynomial/util/TestFunctionCall.h>
#include <test/libpolynomial/PolynomialExecutionFramework.h>
#include <test/libpolynomial/AnalysisFramework.h>
#include <test/TestCase.h>
#include <liblangutil/Exceptions.h>
#include <libdevcore/AnsiColorized.h>

#include <iosfwd>
#include <string>
#include <vector>
#include <utility>

namespace dev
{
namespace polynomial
{
namespace test
{

/**
 * Class that represents a semantic test (or end-to-end test) and allows running it as part of the
 * boost unit test environment or ipoltest. It reads the Polynomial source and an additional comment
 * section from the given file. This comment section should define a set of functions to be called
 * and an expected result they return after being executed.
 */
class SemanticTest: public PolynomialExecutionFramework, public TestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _options)
	{ return std::make_unique<SemanticTest>(_options.filename, _options.ipcPath, _options.svmVersion); }

	explicit SemanticTest(std::string const& _filename, std::string const& _ipcPath, langutil::SVMVersion _svmVersion);

	bool run(std::ostream& _stream, std::string const& _linePrefix = "", bool _formatted = false) override;
	void printSource(std::ostream &_stream, std::string const& _linePrefix = "", bool _formatted = false) const override;
	void printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix = "") const override;

	/// Instantiates a test file parser that parses the additional comment section at the end of
	/// the input stream \param _stream. Each function call is represented using a `FunctionCallTest`
	/// and added to the list of call to be executed when `run()` is called.
	/// Throws if parsing expectations failed.
	void parseExpectations(std::istream& _stream);

	/// Compiles and deploys currently held source.
	/// Returns true if deployment was successful, false otherwise.
	bool deploy(std::string const& _contractName, u256 const& _value, bytes const& _arguments);

private:
	std::string m_source;
	std::vector<TestFunctionCall> m_tests;
};

}
}
}
