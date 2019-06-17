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

#include <libpolynomial/interface/Version.h>

#include <liblangutil/Exceptions.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/Common.h>
#include <polynomial/BuildInfo.h>
#include <string>

using namespace dev;
using namespace dev::polynomial;
using namespace std;

char const* dev::polynomial::VersionNumber = SOF_PROJECT_VERSION;

string const dev::polynomial::VersionString =
	string(dev::polynomial::VersionNumber) +
	(string(POL_VERSION_PRERELEASE).empty() ? "" : "-" + string(POL_VERSION_PRERELEASE)) +
	(string(POL_VERSION_BUILDINFO).empty() ? "" : "+" + string(POL_VERSION_BUILDINFO));

string const dev::polynomial::VersionStringStrict =
	string(dev::polynomial::VersionNumber) +
	(string(POL_VERSION_PRERELEASE).empty() ? "" : "-" + string(POL_VERSION_PRERELEASE)) +
	(string(POL_VERSION_COMMIT).empty() ? "" : "+" + string(POL_VERSION_COMMIT));

bytes const dev::polynomial::VersionCompactBytes = {
	SOF_PROJECT_VERSION_MAJOR,
	SOF_PROJECT_VERSION_MINOR,
	SOF_PROJECT_VERSION_PATCH
};

bool const dev::polynomial::VersionIsRelease = string(POL_VERSION_PRERELEASE).empty();
