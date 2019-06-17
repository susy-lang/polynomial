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
 * @author Christian <c@sofdev.com>
 * @date 2014
 * Formatting functions for errors referencing positions and locations in the source.
 */

#pragma once

#include <ostream>
#include <sstream>
#include <functional>
#include <libsvmasm/SourceLocation.h>

namespace dev
{

struct Exception; // forward

namespace polynomial
{

class Scanner; // forward
class CompilerStack; // forward

struct SourceReferenceFormatter
{
public:
	using ScannerFromSourceNameFun = std::function<Scanner const&(std::string const&)>;
	/// Prints source location if it is given.
	static void printSourceLocation(
		std::ostream& _stream,
		SourceLocation const* _location,
		ScannerFromSourceNameFun const& _scannerFromSourceName
	);
	static void printExceptionInformation(
		std::ostream& _stream,
		Exception const& _exception,
		std::string const& _name,
		ScannerFromSourceNameFun const& _scannerFromSourceName
	);
	static std::string formatExceptionInformation(
		Exception const& _exception,
		std::string const& _name,
		ScannerFromSourceNameFun const& _scannerFromSourceName
	)
	{
		std::ostringstream errorOutput;
		printExceptionInformation(errorOutput, _exception, _name, _scannerFromSourceName);
		return errorOutput.str();
	}
private:
	/// Prints source name if location is given.
	static void printSourceName(
		std::ostream& _stream,
		SourceLocation const* _location,
		ScannerFromSourceNameFun const& _scannerFromSourceName
	);
};

}
}
