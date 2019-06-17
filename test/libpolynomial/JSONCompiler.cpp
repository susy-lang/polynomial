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
 * Unit tests for polc/jsonCompiler.cpp.
 */

#include <string>
#include <iostream>
#include <regex>
#include <boost/test/unit_test.hpp>
#include <libdevcore/JSON.h>
#include <libpolynomial/interface/Version.h>

#include "../Metadata.h"
#include "../TestHelper.h"

using namespace std;

extern "C"
{
extern char const* version();
extern char const* license();
extern char const* compileJSON(char const* _input, bool _optimize);
extern char const* compileJSONMulti(char const* _input, bool _optimize);
extern char const* compileJSONCallback(char const* _input, bool _optimize, void* _readCallback);
extern char const* compileStandard(char const* _input, void* _readCallback);
}

namespace dev
{
namespace polynomial
{
namespace test
{

namespace
{

Json::Value compileSingle(string const& _input)
{
	string output(compileJSON(_input.c_str(), dev::test::Options::get().optimize));
	Json::Value ret;
	BOOST_REQUIRE(Json::Reader().parse(output, ret, false));
	return ret;
}

Json::Value compileMulti(string const& _input, bool _callback)
{
	string output(
		_callback ?
		compileJSONCallback(_input.c_str(), dev::test::Options::get().optimize, NULL) :
		compileJSONMulti(_input.c_str(), dev::test::Options::get().optimize)
	);
	Json::Value ret;
	BOOST_REQUIRE(Json::Reader().parse(output, ret, false));
	return ret;
}

Json::Value compile(string const& _input)
{
	string output(compileStandard(_input.c_str(), NULL));
	Json::Value ret;
	BOOST_REQUIRE(Json::Reader().parse(output, ret, false));
	return ret;
}

} // end anonymous namespace

BOOST_AUTO_TEST_SUITE(JSONCompiler)

BOOST_AUTO_TEST_CASE(read_version)
{
	string output(version());
	BOOST_CHECK(output.find(VersionString) == 0);
}

BOOST_AUTO_TEST_CASE(read_license)
{
	string output(license());
	BOOST_CHECK(output.find("GNU GENERAL PUBLIC LICENSE") != string::npos);
}

BOOST_AUTO_TEST_CASE(basic_compilation)
{
	char const* input = R"(
	{
		"sources": {
			"fileA": "contract A { }"
		}
	}
	)";
	Json::Value result = compileMulti(input, false);
	BOOST_CHECK(result.isObject());

	// Compare with compileJSONCallback
	BOOST_CHECK_EQUAL(
		dev::jsonCompactPrint(result),
		dev::jsonCompactPrint(compileMulti(input, true))
	);

	BOOST_CHECK(result["contracts"].isObject());
	BOOST_CHECK(result["contracts"]["fileA:A"].isObject());
	Json::Value contract = result["contracts"]["fileA:A"];
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["interface"].isString());
	BOOST_CHECK_EQUAL(contract["interface"].asString(), "[]");
	BOOST_CHECK(contract["bytecode"].isString());
	BOOST_CHECK_EQUAL(
		dev::test::bytecodeSansMetadata(contract["bytecode"].asString()),
		"60606040523415600e57600080fd5b5b603680601c6000396000f30060606040525b600080fd00"
	);
	BOOST_CHECK(contract["runtimeBytecode"].isString());
	BOOST_CHECK_EQUAL(
		dev::test::bytecodeSansMetadata(contract["runtimeBytecode"].asString()),
		"60606040525b600080fd00"
	);
	BOOST_CHECK(contract["functionHashes"].isObject());
	BOOST_CHECK(contract["gasEstimates"].isObject());
	BOOST_CHECK_EQUAL(
		dev::jsonCompactPrint(contract["gasEstimates"]),
		"{\"creation\":[62,10800],\"external\":{},\"internal\":{}}"
	);
	BOOST_CHECK(contract["metadata"].isString());
	BOOST_CHECK(dev::test::isValidMetadata(contract["metadata"].asString()));
	BOOST_CHECK(result["sources"].isObject());
	BOOST_CHECK(result["sources"]["fileA"].isObject());
	BOOST_CHECK(result["sources"]["fileA"]["AST"].isObject());
	BOOST_CHECK_EQUAL(
		dev::jsonCompactPrint(result["sources"]["fileA"]["AST"]),
		"{\"attributes\":{\"absolutePath\":\"fileA\",\"exportedSymbols\":{\"A\":[1]}},"
		"\"children\":[{\"attributes\":{\"baseContracts\":[null],\"contractDependencies\":[null],"
		"\"contractKind\":\"contract\",\"documentation\":null,\"fullyImplemented\":true,\"linearizedBaseContracts\":[1],"
		"\"name\":\"A\",\"nodes\":[null],\"scope\":2},\"id\":1,\"name\":\"ContractDefinition\","
		"\"src\":\"0:14:0\"}],\"id\":2,\"name\":\"SourceUnit\",\"src\":\"0:14:0\"}"
	);
}

BOOST_AUTO_TEST_CASE(single_compilation)
{
	Json::Value result = compileSingle("contract A { }");
	BOOST_CHECK(result.isObject());

	BOOST_CHECK(result["contracts"].isObject());
	BOOST_CHECK(result["contracts"][":A"].isObject());
	Json::Value contract = result["contracts"][":A"];
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["interface"].isString());
	BOOST_CHECK_EQUAL(contract["interface"].asString(), "[]");
	BOOST_CHECK(contract["bytecode"].isString());
	BOOST_CHECK_EQUAL(
		dev::test::bytecodeSansMetadata(contract["bytecode"].asString()),
		"60606040523415600e57600080fd5b5b603680601c6000396000f30060606040525b600080fd00"
	);
	BOOST_CHECK(contract["runtimeBytecode"].isString());
	BOOST_CHECK_EQUAL(
		dev::test::bytecodeSansMetadata(contract["runtimeBytecode"].asString()),
		"60606040525b600080fd00"
	);
	BOOST_CHECK(contract["functionHashes"].isObject());
	BOOST_CHECK(contract["gasEstimates"].isObject());
	BOOST_CHECK_EQUAL(
		dev::jsonCompactPrint(contract["gasEstimates"]),
		"{\"creation\":[62,10800],\"external\":{},\"internal\":{}}"
	);
	BOOST_CHECK(contract["metadata"].isString());
	BOOST_CHECK(dev::test::isValidMetadata(contract["metadata"].asString()));
	BOOST_CHECK(result["sources"].isObject());
	BOOST_CHECK(result["sources"][""].isObject());
	BOOST_CHECK(result["sources"][""]["AST"].isObject());
	BOOST_CHECK_EQUAL(
		dev::jsonCompactPrint(result["sources"][""]["AST"]),
		"{\"attributes\":{\"absolutePath\":\"\",\"exportedSymbols\":{\"A\":[1]}},"
		"\"children\":[{\"attributes\":{\"baseContracts\":[null],\"contractDependencies\":[null],"
		"\"contractKind\":\"contract\",\"documentation\":null,\"fullyImplemented\":true,\"linearizedBaseContracts\":[1],"
		"\"name\":\"A\",\"nodes\":[null],\"scope\":2},\"id\":1,\"name\":\"ContractDefinition\","
		"\"src\":\"0:14:0\"}],\"id\":2,\"name\":\"SourceUnit\",\"src\":\"0:14:0\"}"
	);
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
	BOOST_CHECK(result.isMember("contracts"));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces