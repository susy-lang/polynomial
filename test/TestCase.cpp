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

#include <libdevcore/StringUtils.h>

#include <boost/algorithm/cxx11/none_of.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/range/adaptor/map.hpp>

#include <stdexcept>

#include <iostream>

using namespace dev;
using namespace polynomial;
using namespace dev::polynomial::test;
using namespace std;

void TestCase::printUpdatedSettings(ostream& _stream, const string& _linePrefix, const bool)
{
	if (m_validatedSettings.empty())
		return;

	_stream << _linePrefix << "// ====" << endl;
	for (auto const& setting: m_validatedSettings)
		_stream << _linePrefix << "// " << setting.first << ": " << setting.second << endl;
}

bool TestCase::isTestFilename(boost::filesystem::path const& _filename)
{
	string extension = _filename.extension().string();
	return (extension == ".pol" || extension == ".yul") &&
		   !boost::starts_with(_filename.string(), "~") &&
			!boost::starts_with(_filename.string(), ".");
}

bool TestCase::validateSettings(langutil::SVMVersion)
{
	if (!m_settings.empty())
		throw runtime_error(
			"Unknown setting(s): " +
			joinHumanReadable(m_settings | boost::adaptors::map_keys)
		);
	return true;
}

string TestCase::parseSourceAndSettings(istream& _stream)
{
	string source;
	string line;
	static string const comment("// ");
	static string const settingsDelimiter("// ====");
	static string const delimiter("// ----");
	bool sourcePart = true;
	while (getline(_stream, line))
	{
		if (boost::algorithm::starts_with(line, delimiter))
			break;
		else if (boost::algorithm::starts_with(line, settingsDelimiter))
			sourcePart = false;
		else if (sourcePart)
			source += line + "\n";
		else if (boost::algorithm::starts_with(line, comment))
		{
			size_t colon = line.find(':');
			if (colon == string::npos)
				throw runtime_error(string("Expected \":\" inside setting."));
			string key = line.substr(comment.size(), colon - comment.size());
			string value = line.substr(colon + 1);
			boost::algorithm::trim(key);
			boost::algorithm::trim(value);
			m_settings[key] = value;
		}
		else
			throw runtime_error(string("Expected \"//\" or \"// ---\" to terminate settings and source."));
	}
	return source;
}

void TestCase::expect(string::iterator& _it, string::iterator _end, string::value_type _c)
{
	if (_it == _end || *_it != _c)
		throw runtime_error(string("Invalid test expectation. Expected: \"") + _c + "\".");
	++_it;
}

bool SVMVersionRestrictedTestCase::validateSettings(langutil::SVMVersion _svmVersion)
{
	if (!m_settings.count("SVMVersion"))
		return true;

	string versionString = m_settings["SVMVersion"];
	m_validatedSettings["SVMVersion"] = versionString;
	m_settings.erase("SVMVersion");

	if (!TestCase::validateSettings(_svmVersion))
		return false;

	if (versionString.empty())
		return true;

	string comparator;
	size_t versionBegin = 0;
	for (auto character: versionString)
		if (!isalpha(character))
		{
			comparator += character;
			versionBegin++;
		}
		else
			break;

	versionString = versionString.substr(versionBegin);
	boost::optional<langutil::SVMVersion> version = langutil::SVMVersion::fromString(versionString);
	if (!version)
		throw runtime_error("Invalid SVM version: \"" + versionString + "\"");

	if (comparator == ">")
		return _svmVersion > version;
	else if (comparator == ">=")
		return _svmVersion >= version;
	else if (comparator == "<")
		return _svmVersion < version;
	else if (comparator == "<=")
		return _svmVersion <= version;
	else if (comparator == "=")
		return _svmVersion == version;
	else if (comparator == "!")
		return !(_svmVersion == version);
	else
		throw runtime_error("Invalid SVM comparator: \"" + comparator + "\"");
	return false; // not reached
}
