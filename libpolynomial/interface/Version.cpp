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
 * @date 2015
 * Versioning.
 */

#include <libpolynomial/interface/Version.h>
#include <string>
#include <libdevcore/CommonData.h>
#include <libdevcore/Common.h>
#include <libpolynomial/interface/Utils.h>
#include <polynomial/BuildInfo.h>

using namespace dev;
using namespace dev::polynomial;
using namespace std;

char const* dev::polynomial::VersionNumber = SOF_PROJECT_VERSION;

string const dev::polynomial::VersionString =
	string(dev::polynomial::VersionNumber) +
	(string(POL_VERSION_PRERELEASE).empty() ? "" : "-" + string(POL_VERSION_PRERELEASE)) +
	(string(POL_VERSION_BUILDINFO).empty() ? "" : "+" + string(POL_VERSION_BUILDINFO));


bytes dev::polynomial::binaryVersion()
{
	bytes ret{0};
	size_t i = 0;
	auto parseDecimal = [&]()
	{
		size_t ret = 0;
		polAssert('0' <= VersionString[i] && VersionString[i] <= '9', "");
		for (; i < VersionString.size() && '0' <= VersionString[i] && VersionString[i] <= '9'; ++i)
			ret = ret * 10 + (VersionString[i] - '0');
		return ret;
	};
	ret.push_back(byte(parseDecimal()));
	polAssert(i < VersionString.size() && VersionString[i] == '.', "");
	++i;
	ret.push_back(byte(parseDecimal()));
	polAssert(i < VersionString.size() && VersionString[i] == '.', "");
	++i;
	ret.push_back(byte(parseDecimal()));
	polAssert(i < VersionString.size() && VersionString[i] == '-', "");
	++i;
	polAssert(i + 7 < VersionString.size(), "");
	bytes commitHash = fromHex(VersionString.substr(i, 8));
	if (commitHash.empty())
		commitHash = bytes(4, 0);
	ret += commitHash;
	polAssert(ret.size() == 1 + 3 + 4, "");

	return ret;
}

