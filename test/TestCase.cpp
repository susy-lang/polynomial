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

#include <test/TestCase.h>

#include <boost/algorithm/cxx11/none_of.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <stdexcept>

using namespace dev;
using namespace polynomial;
using namespace dev::polynomial::test;
using namespace std;

bool TestCase::isTestFilename(boost::filesystem::path const& _filename)
{
	string extension = _filename.extension().string();
	return (extension == ".pol" || extension == ".yul") &&
		   !boost::starts_with(_filename.string(), "~") &&
		   !boost::starts_with(_filename.string(), ".");
}

bool TestCase::supportedForSVMVersion(langutil::SVMVersion _svmVersion) const
{
	return boost::algorithm::none_of(m_svmVersionRules, [&](auto const& rule) { return !rule(_svmVersion); });
}

string TestCase::parseSource(istream& _stream)
{
	string source;
	string line;
	static string const delimiter("// ----");
	static string const svmVersion("// SVMVersion: ");
	bool isTop = true;
	while (getline(_stream, line))
		if (boost::algorithm::starts_with(line, delimiter))
			break;
		else
		{
			if (isTop && boost::algorithm::starts_with(line, svmVersion))
			{
				string versionString = line.substr(svmVersion.size() + 1);
				auto version = langutil::SVMVersion::fromString(versionString);
				if (!version)
					throw runtime_error("Invalid SVM version: \"" + versionString + "\"");
				switch (line.at(svmVersion.size()))
				{
					case '>':
						m_svmVersionRules.emplace_back([version](langutil::SVMVersion _version) {
							return version < _version;
						});
						break;
					case '<':
						m_svmVersionRules.emplace_back([version](langutil::SVMVersion _version) {
							return _version < version;
						});
						break;
					case '=':
						m_svmVersionRules.emplace_back([version](langutil::SVMVersion _version) {
							return _version == version;
						});
						break;
					case '!':
						m_svmVersionRules.emplace_back([version](langutil::SVMVersion _version) {
							return !(_version == version);
						});
						break;
				}
			}
			else
				isTop = false;
			source += line + "\n";
		}
	return source;
}

void TestCase::expect(string::iterator& _it, string::iterator _end, string::value_type _c)
{
	if (_it == _end || *_it != _c)
		throw runtime_error(string("Invalid test expectation. Expected: \"") + _c + "\".");
	++_it;
}
