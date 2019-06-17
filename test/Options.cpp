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
* @author Marko Simovic <markobarko@gmail.com>
* @date 2014
*/

#include <test/Options.h>

#include <test/Common.h>

#include <liblangutil/SVMVersion.h>
#include <liblangutil/Exceptions.h>

#include <boost/test/framework.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace dev::test;
namespace fs = boost::filesystem;

Options const& Options::get()
{
	static Options instance;
	return instance;
}

Options::Options()
{
	auto const& suite = boost::unit_test::framework::master_test_suite();
	for (auto i = 0; i < suite.argc; i++)
		if (string(suite.argv[i]) == "--ipcpath" && i + 1 < suite.argc)
		{
			ipcPath = suite.argv[i + 1];
			i++;
		}
		else if (string(suite.argv[i]) == "--testpath" && i + 1 < suite.argc)
		{
			testPath = suite.argv[i + 1];
			i++;
		}
		else if (string(suite.argv[i]) == "--optimize")
			optimize = true;
		else if (string(suite.argv[i]) == "--svm-version")
		{
			svmVersionString = i + 1 < suite.argc ? suite.argv[i + 1] : "INVALID";
			++i;
		}
		else if (string(suite.argv[i]) == "--show-messages")
			showMessages = true;
		else if (string(suite.argv[i]) == "--no-ipc")
			disableIPC = true;
		else if (string(suite.argv[i]) == "--no-smt")
			disableSMT = true;

	if (!disableIPC && ipcPath.empty())
		if (auto path = getenv("SOF_TEST_IPC"))
			ipcPath = path;

	if (testPath.empty())
		if (auto path = getenv("SOF_TEST_PATH"))
			testPath = path;

	if (testPath.empty())
		testPath = discoverTestPath();
}

void Options::validate() const
{
	polAssert(
		!dev::test::Options::get().testPath.empty(),
		"No test path specified. The --testpath argument is required."
	);
	if (!disableIPC)
		polAssert(
			!dev::test::Options::get().ipcPath.empty(),
			"No ipc path specified. The --ipcpath argument is required, unless --no-ipc is used."
		);
}

dev::polynomial::SVMVersion Options::svmVersion() const
{
	if (!svmVersionString.empty())
	{
		// We do this check as opposed to in the constructor because the BOOST_REQUIRE
		// macros cannot yet be used in the constructor.
		auto version = polynomial::SVMVersion::fromString(svmVersionString);
		BOOST_REQUIRE_MESSAGE(version, "Invalid SVM version: " + svmVersionString);
		return *version;
	}
	else
		return dev::polynomial::SVMVersion();
}
