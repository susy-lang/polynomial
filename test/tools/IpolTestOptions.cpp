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
/** @file IpolTestOptions.cpp
* @date 2019
*/

#include <test/tools/IpolTestOptions.h>
#include <boost/filesystem.hpp>
#include <string>
#include <iostream>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace dev
{
namespace test
{

auto const description = R"(ipoltest, tool for interactively managing test contracts.
Usage: ipoltest [Options] --ipcpath ipcpath
Interactively validates test contracts.

Allowed options)";

std::string editorPath()
{
	if (getenv("EDITOR"))
		return getenv("EDITOR");
	else if (fs::exists("/usr/bin/editor"))
		return "/usr/bin/editor";

	return std::string{};
}

IpolTestOptions::IpolTestOptions(std::string* _editor):
	CommonOptions(description)
{
	options.add_options()
		("help", po::bool_switch(&showHelp), "Show this help screen.")
		("no-color", po::bool_switch(&noColor), "don't use colors")
		("editor", po::value<std::string>(_editor)->default_value(editorPath()), "editor for opening test files");

}

bool IpolTestOptions::parse(int _argc, char const* const* _argv)
{
	bool const res = CommonOptions::parse(_argc, _argv);

	if (showHelp || !res)
	{
		std::cout << options << std::endl;
		return false;
	}

	return res;
}

}
}
