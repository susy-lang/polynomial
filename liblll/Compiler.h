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
/** @file Compiler.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <libdevcore/Common.h>

#include <liblangutil/SVMVersion.h>

#include <string>
#include <vector>

namespace dev
{
namespace lll
{

using ReadCallback = std::function<std::string(std::string const&)>;

std::string parseLLL(std::string _src);
std::string compileLLLToAsm(std::string _src, langutil::SVMVersion _svmVersion, bool _opt = true, std::vector<std::string>* _errors = nullptr, ReadCallback const& _readFile = ReadCallback());
bytes compileLLL(std::string _src, langutil::SVMVersion _svmVersion, bool _opt = true, std::vector<std::string>* _errors = nullptr, ReadCallback const& _readFile = ReadCallback());

}
}
