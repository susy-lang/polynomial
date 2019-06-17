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
/** @file IpolTestOptions.h
 */

#pragma once

#include <liblangutil/SVMVersion.h>

#include <test/Common.h>

namespace dev
{
namespace test
{

struct IpolTestOptions: CommonOptions
{
	bool showHelp = false;
	bool noColor = false;
	std::string testFilter = std::string{};

	IpolTestOptions(std::string* _editor);
	bool parse(int _argc, char const* const* _argv) override;
	void validate() const override;
};
}
}
