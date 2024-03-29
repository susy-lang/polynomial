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
#include <test/TestCase.h>

#include <libpolynomial/ast/Types.h>
#include <liblangutil/Exceptions.h>
#include <libdevcore/AnsiColorized.h>
#include <libdevcore/CommonData.h>

#include <json/json.h>

#include <iosfwd>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace dev
{
namespace polynomial
{
namespace test
{

/**
 * Representation of a notice, warning or error that can occur while
 * formatting and therefore updating an interactive function call test.
 */
struct FormatError
{
	enum Type
	{
		Notice,
		Warning,
		Error
	};

	explicit FormatError(Type _type, std::string _message):
		type(_type),
		message(std::move(_message))
	{}

	Type type;
	std::string message;
};
using FormatErrors = std::vector<FormatError>;

/**
 * Utility class that collects notices, warnings and errors and is able
 * to format them for ANSI colorized output during the interactive update
 * process in ipoltest.
 * Its purpose is to help users of ipoltest to automatically
 * update test files and always keep track of what is happening.
 */
class ErrorReporter
{
public:
	explicit ErrorReporter() {}

	/// Adds a new FormatError of type Notice with the given message.
	void notice(std::string _notice)
	{
		m_errors.push_back(FormatError{FormatError::Notice, std::move(_notice)});
	}

	/// Adds a new FormatError of type Warning with the given message.
	void warning(std::string _warning)
	{
		m_errors.push_back(FormatError{FormatError::Warning, std::move(_warning)});
	}

	/// Adds a new FormatError of type Error with the given message.
	void error(std::string _error)
	{
		m_errors.push_back(FormatError{FormatError::Error, std::move(_error)});
	}

	/// Prints all errors depending on their type using ANSI colorized output.
	/// It will be used to print notices, warnings and errors during the
	/// interactive update process.
	std::string format(std::string const& _linePrefix, bool _formatted)
	{
		std::stringstream os;
		for (auto const& error: m_errors)
		{
			switch (error.type)
			{
			case FormatError::Notice:

				break;
			case FormatError::Warning:
				AnsiColorized(
					os,
					_formatted,
					{formatting::YELLOW}
				) << _linePrefix << "Warning: " << error.message << std::endl;
				break;
			case FormatError::Error:
				AnsiColorized(
					os,
					_formatted,
					{formatting::RED}
				) << _linePrefix << "Error: " << error.message << std::endl;
				break;
			}
		}
		return os.str();
	}

private:
	FormatErrors m_errors;
};

/**
 * Represents a function call and the result it returned. It stores the call
 * representation itself, the actual byte result (if any) and a string representation
 * used for the interactive update routine provided by ipoltest. It also provides
 * functionality to compare the actual result with the expectations attached to the
 * call object, as well as a way to reset the result if executed multiple times.
 */
class TestFunctionCall
{
public:
	TestFunctionCall(FunctionCall _call): m_call(std::move(_call)) {}

	/// Formats this function call test and applies the format that was detected during parsing.
	/// If _renderResult is false, the expected result of the call will is used, if it's false
	/// the actual result is used.
	/// If _highlight is false, it's formatted without colorized highlighting. If it's true, AnsiColorized is
	/// used to apply a colorized highlighting.
	/// If test expectations do not match, the contract ABI is consulted in order to get the
	/// right encoding for returned bytes, based on the parsed return types.
	/// Reports warnings and errors to the error reporter.
	std::string format(
		ErrorReporter& _errorReporter,
		std::string const& _linePrefix = "",
		bool const _renderResult = false,
		bool const _highlight = false
	) const;

	/// Overloaded version that passes an error reporter which is never used outside
	/// of this function.
	std::string format(
		std::string const& _linePrefix = "",
		bool const _renderResult = false,
		bool const _highlight = false
	) const
	{
		ErrorReporter reporter;
		return format(reporter, _linePrefix, _renderResult, _highlight);
	}

	/// Resets current results in case the function was called and the result
	/// stored already (e.g. if test case was updated via ipoltest).
	void reset();

	FunctionCall const& call() const { return m_call; }
	void setFailure(const bool _failure) { m_failure = _failure; }
	void setRawBytes(const bytes _rawBytes) { m_rawBytes = _rawBytes; }
	void setContractABI(Json::Value _contractABI) { m_contractABI = std::move(_contractABI); }

private:
	/// Tries to format the given `bytes`, applying the detected ABI types that have be set for each parameter.
	/// Throws if there's a mismatch in the size of `bytes` and the desired formats that are specified
	/// in the ABI type.
	/// Reports warnings and errors to the error reporter.
	std::string formatBytesParameters(
		ErrorReporter& _errorReporter,
		bytes const& _bytes,
		std::string const& _signature,
		ParameterList const& _params,
		bool highlight = false
	) const;

	/// Formats a given _bytes applying the _abiType.
	std::string formatBytesRange(
		bytes const& _bytes,
		ABIType const& _abiType
	) const;

	/// Formats the given parameters using their raw string representation.
	std::string formatRawParameters(
		ParameterList const& _params,
		std::string const& _linePrefix = ""
	) const;

	/// Compares raw expectations (which are converted to a byte representation before),
	/// and also the expected transaction status of the function call to the actual test results.
	bool matchesExpectation() const;

	/// Function call that has been parsed and which holds all parameters / expectations.
	FunctionCall m_call;
	/// Result of the actual call been made.
	bytes m_rawBytes = bytes{};
	/// Transaction status of the actual call. False in case of a REVERT or any other failure.
	bool m_failure = true;
	/// JSON object which holds the contract ABI and that is used to set the output formatting
	/// in the interactive update routine.
	Json::Value m_contractABI;
};

}
}
}
