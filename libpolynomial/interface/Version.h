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
 * @date 2015
 * Versioning.
 */

#pragma once

#include <libdevcore/Common.h>
#include <string>

namespace dev
{
namespace polynomial
{

extern char const* VersionNumber;
extern std::string const VersionString;
extern std::string const VersionStringStrict;
extern bytes const VersionCompactBytes;
extern bool const VersionIsRelease;

}
}
