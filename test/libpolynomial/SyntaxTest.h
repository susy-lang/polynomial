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

struct SyntaxTestError
{
	std::string type;
	std::string message;
	int locationStart;
	int locationEnd;
	bool operator==(SyntaxTestError const& _rhs) const
	{
		return type == _rhs.type &&
			message == _rhs.message &&
			locationStart == _rhs.locationStart &&
			locationEnd == _rhs.locationEnd;
	}
};


class SyntaxTest: AnalysisFramework, public SVMVersionRestrictedTestCase
{
public:
	static std::unique_ptr<TestCase> create(Config const& _config)
	{ return std::make_unique<SyntaxTest>(_config.filename, _config.svmVersion); }
	SyntaxTest(std::string const& _filename, langutil::SVMVersion _svmVersion);

	TestResult run(std::ostream& _stream, std::string const& _linePrefix = "", bool _formatted = false) override;

	void printSource(std::ostream &_stream, std::string const &_linePrefix = "", bool _formatted = false) const override;
	void printUpdatedExpectations(std::ostream& _stream, std::string const& _linePrefix) const override
	{
		if (!m_errorList.empty())
			printErrorList(_stream, m_errorList, _linePrefix, false);
	}

	static std::string errorMessage(Exception const& _e);
protected:
	static void printErrorList(
		std::ostream& _stream,
		std::vector<SyntaxTestError> const& _errors,
		std::string const& _linePrefix,
		bool _formatted = false
	);

	virtual bool printExpectationAndError(std::ostream& _stream, std::string const& _linePrefix = "", bool _formatted = false);

	static std::vector<SyntaxTestError> parseExpectations(std::istream& _stream);

	std::string m_source;
	std::vector<SyntaxTestError> m_expectations;
	std::vector<SyntaxTestError> m_errorList;
	bool m_optimiseYul = false;
	langutil::SVMVersion const m_svmVersion;
};

}
}
}
