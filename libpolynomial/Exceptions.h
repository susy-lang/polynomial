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
 * Polynomial exception hierarchy.
 */

#pragma once

#include <string>
#include <utility>
#include <libdevcore/Exceptions.h>
#include <libsvmasm/SourceLocation.h>

namespace dev
{
namespace polynomial
{
struct Error: virtual Exception {};

struct ParserError: virtual Error {};
struct TypeError: virtual Error {};
struct DeclarationError: virtual Error {};
struct DocstringParsingError: virtual Error {};
struct Warning: virtual Error {};

struct CompilerError: virtual Exception {};
struct InternalCompilerError: virtual Exception {};
struct FatalError: virtual Exception {};

using errorSourceLocationInfo = std::pair<std::string, SourceLocation>;

class SecondarySourceLocation
{
public:
	SecondarySourceLocation& append(std::string const& _errMsg, SourceLocation const& _sourceLocation)
	{
		infos.push_back(std::make_pair(_errMsg, _sourceLocation));
		return *this;
	}
	std::vector<errorSourceLocationInfo> infos;
};

using errinfo_sourceLocation = boost::error_info<struct tag_sourceLocation, SourceLocation>;
using errinfo_secondarySourceLocation = boost::error_info<struct tag_secondarySourceLocation, SecondarySourceLocation>;

}
}
