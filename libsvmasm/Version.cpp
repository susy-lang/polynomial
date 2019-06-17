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

#include <string>
#include <sophon/BuildInfo.h>
#include <libdevcore/Common.h>
#include <libsvmasm/Version.h>

using namespace dev;
using namespace std;

char const* dev::sof::VersionNumberLibSvmAsm = SOF_PROJECT_VERSION;
extern string const dev::sof::VersionStringLibSvmAsm =
	string(dev::sof::VersionNumberLibSvmAsm) +
	"-" +
	string(DEV_QUOTED(SOF_COMMIT_HASH)).substr(0, 8) +
	(SOF_CLEAN_REPO ? "" : "*") +
	"/" DEV_QUOTED(SOF_BUILD_TYPE) "-" DEV_QUOTED(SOF_BUILD_PLATFORM);

