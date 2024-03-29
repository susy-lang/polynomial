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
/**
 * @author Rhett <roadriverrail@gmail.com>
 * @date 2017
 * Error reporting helper class.
 */

#pragma once

#include <libdevcore/CommonData.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceLocation.h>
#include <libdevcore/StringUtils.h>

#include <boost/range/adaptor/filtered.hpp>

namespace langutil
{

class ErrorReporter
{
public:

	explicit ErrorReporter(ErrorList& _errors):
		m_errorList(_errors) { }

	ErrorReporter(ErrorReporter const& _errorReporter) noexcept:
		m_errorList(_errorReporter.m_errorList) { }

	ErrorReporter& operator=(ErrorReporter const& _errorReporter);

	void append(ErrorList const& _errorList)
	{
		m_errorList += _errorList;
	}

	void warning(std::string const& _description);

	void warning(SourceLocation const& _location, std::string const& _description);

	void warning(
		SourceLocation const& _location,
		std::string const& _description,
		SecondarySourceLocation const& _secondaryLocation
	);

	void error(
		Error::Type _type,
		SourceLocation const& _location,
		std::string const& _description
	);

	void declarationError(
		SourceLocation const& _location,
		SecondarySourceLocation const& _secondaryLocation,
		std::string const& _description
	);

	void declarationError(SourceLocation const& _location, std::string const& _description);

	void fatalDeclarationError(SourceLocation const& _location, std::string const& _description);

	void parserError(SourceLocation const& _location, std::string const& _description);

	void fatalParserError(SourceLocation const& _location, std::string const& _description);

	void syntaxError(SourceLocation const& _location, std::string const& _description);

	void typeError(
		SourceLocation const& _location,
		SecondarySourceLocation const& _secondaryLocation = SecondarySourceLocation(),
		std::string const& _description = std::string()
	);

	void typeError(SourceLocation const& _location, std::string const& _description);

	template <typename... Strings>
	void typeErrorConcatenateDescriptions(SourceLocation const& _location, Strings const&... _descriptions)
	{
		std::initializer_list<std::string> const descs = {_descriptions...};
		polAssert(descs.size() > 0, "Need error descriptions!");

		auto filterEmpty = boost::adaptors::filtered([](std::string const& _s) { return !_s.empty(); });

		std::string errorStr = dev::joinHumanReadable(descs | filterEmpty, " ");

		error(Error::Type::TypeError, _location, errorStr);
	}

	void fatalTypeError(SourceLocation const& _location, std::string const& _description);
	void fatalTypeError(SourceLocation const& _location, SecondarySourceLocation const& _secondLocation, std::string const& _description);

	void docstringParsingError(std::string const& _description);

	ErrorList const& errors() const;

	void clear();

	/// @returns true iff there is any error (ignores warnings).
	bool hasErrors() const
	{
		return m_errorCount > 0;
	}

private:
	void error(
		Error::Type _type,
		SourceLocation const& _location,
		SecondarySourceLocation const& _secondaryLocation,
		std::string const& _description = std::string());

	void fatalError(
		Error::Type _type,
		SourceLocation const& _location,
		SecondarySourceLocation const& _secondaryLocation,
		std::string const& _description = std::string());

	void fatalError(
		Error::Type _type,
		SourceLocation const& _location = SourceLocation(),
		std::string const& _description = std::string());

	// @returns true if error shouldn't be stored
	bool checkForExcessiveErrors(Error::Type _type);

	ErrorList& m_errorList;

	unsigned m_errorCount = 0;
	unsigned m_warningCount = 0;

	unsigned const c_maxWarningsAllowed = 256;
	unsigned const c_maxErrorsAllowed = 256;
};

}

