/*
	This file is part of cpp-sophon.

	cpp-sophon is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-sophon is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MSRCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-sophon.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@sofdev.com>
 * @date 2014
 * Formatting functions for errors referencing positions and locations in the source.
 */

#pragma once

#include <ostream>
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
	static void printSourceLocation(std::ostream& _stream, SourceLocation const& _location, Scanner const& _scanner);
	static void printExceptionInformation(
		std::ostream& _stream,
		Exception const& _exception,
		std::string const& _name,
		std::function<Scanner const&(std::string const&)> const& _scannerFromSourceName
	);
private:
	static void printSourceName(std::ostream& _stream, SourceLocation const& _location, Scanner const& _scanner);
};

}
}
