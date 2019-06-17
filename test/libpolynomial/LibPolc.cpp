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
 * @date 2017
 * Unit tests for libpolc/libpolc.cpp.
 */

#include <string>
#include <boost/test/unit_test.hpp>
#include <libdevcore/JSON.h>
#include <libpolynomial/interface/Version.h>
#include <libpolc/libpolc.h>

using namespace std;

namespace dev
{
namespace polynomial
{
namespace test
{

namespace
{

/// TODO: share this between StandardCompiler.cpp
/// Helper to match a specific error type and message
bool containsError(Json::Value const& _compilerResult, string const& _type, string const& _message)
{
	if (!_compilerResult.isMember("errors"))
		return false;

	for (auto const& error: _compilerResult["errors"])
	{
		BOOST_REQUIRE(error.isObject());
		BOOST_REQUIRE(error["type"].isString());
		BOOST_REQUIRE(error["message"].isString());
		if ((error["type"].asString() == _type) && (error["message"].asString() == _message))
			return true;
	}

	return false;
}

Json::Value compile(string const& _input, CStyleReadFileCallback _callback = nullptr)
{
	string output(polynomial_compile(_input.c_str(), _callback));
	Json::Value ret;
	BOOST_REQUIRE(jsonParseStrict(output, ret));
	polynomial_free();
	return ret;
}

} // end anonymous namespace

BOOST_AUTO_TEST_SUITE(LibPolc)

BOOST_AUTO_TEST_CASE(read_version)
{
	string output(polynomial_version());
	BOOST_CHECK(output.find(VersionString) == 0);
	polynomial_free();
}

BOOST_AUTO_TEST_CASE(read_license)
{
	string output(polynomial_license());
	BOOST_CHECK(output.find("GNU GENERAL PUBLIC LICENSE") != string::npos);
	polynomial_free();
}

BOOST_AUTO_TEST_CASE(standard_compilation)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"sources": {
			"fileA": {
				"content": "contract A { }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(result.isObject());

	// Only tests some assumptions. The StandardCompiler is tested properly in another suite.
	BOOST_CHECK(result.isMember("sources"));
	// This used to test that it is a member, but we did not actually request any output,
	// so there should not be a contract member.
	BOOST_CHECK(!result.isMember("contracts"));
}

BOOST_AUTO_TEST_CASE(missing_callback)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"sources": {
			"fileA": {
				"content": "import \"missing.pol\"; contract A { }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(result.isObject());

	BOOST_CHECK(containsError(result, "ParserError", "Source \"missing.pol\" not found: File not supplied initially."));
}

BOOST_AUTO_TEST_CASE(with_callback)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"sources": {
			"fileA": {
				"content": "import \"found.pol\"; import \"notfound.pol\"; contract A { }"
			}
		}
	}
	)";

	CStyleReadFileCallback callback{
		[](char const* _path, char** o_contents, char** o_error)
		{
			// Caller frees the pointers.
			if (string(_path) == "found.pol")
			{
				static string content{"import \"missing.pol\"; contract B {}"};
				*o_contents = strdup(content.c_str());
				*o_error = nullptr;
			}
			else if (string(_path) == "missing.pol")
			{
				static string errorMsg{"Missing file."};
				*o_error = strdup(errorMsg.c_str());
				*o_contents = nullptr;
			}
			else
			{
				*o_error = nullptr;
				*o_contents = nullptr;
			}
		}
	};

	Json::Value result = compile(input, callback);
	BOOST_CHECK(result.isObject());

	// This ensures that "found.pol" was properly loaded which triggered the second import statement.
	BOOST_CHECK(containsError(result, "ParserError", "Source \"missing.pol\" not found: Missing file."));

	// This should be placed due to the missing "notfound.pol" which sets both pointers to null.
	BOOST_CHECK(containsError(result, "ParserError", "Source \"notfound.pol\" not found: File not found."));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
