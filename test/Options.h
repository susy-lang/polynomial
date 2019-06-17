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
/** @file TestHelper.h
 */

#pragma once

#include <libpolynomial/interface/SVMVersion.h>

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/version.hpp>

#include <functional>

namespace dev
{
namespace test
{

struct Options: boost::noncopyable
{
	std::string ipcPath;
	boost::filesystem::path testPath;
	bool showMessages = false;
	bool optimize = false;
	bool disableIPC = false;
	bool disableSMT = false;

	void validate() const;
	polynomial::SVMVersion svmVersion() const;

	static Options const& get();

private:
	std::string svmVersionString;

	Options();
};

}
}
