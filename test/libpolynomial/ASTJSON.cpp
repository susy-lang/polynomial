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
 * @date 2016
 * Tests for the json ast output.
 */

#include <string>
#include <boost/test/unit_test.hpp>
#include <libpolynomial/interface/Exceptions.h>
#include <libpolynomial/interface/CompilerStack.h>
#include <libpolynomial/ast/ASTJsonConverter.h>

using namespace std;

namespace dev
{
namespace polynomial
{
namespace test
{

BOOST_AUTO_TEST_SUITE(PolynomialASTJSON)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	CompilerStack c;
	c.addSource("a", "contract C {}");
	c.parse();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(c.ast("a"), sourceIndices).json();
	BOOST_CHECK_EQUAL(astJson["name"], "root");
}

BOOST_AUTO_TEST_CASE(source_location)
{
	CompilerStack c;
	c.addSource("a", "contract C { function f() { var x = 2; x++; } }");
	c.parse();
	map<string, unsigned> sourceIndices;
	sourceIndices["a"] = 1;
	Json::Value astJson = ASTJsonConverter(c.ast("a"), sourceIndices).json();
	BOOST_CHECK_EQUAL(astJson["name"], "root");
	BOOST_CHECK_EQUAL(astJson["children"][0]["name"], "Contract");
	BOOST_CHECK_EQUAL(astJson["children"][0]["children"][0]["name"], "Function");
	BOOST_CHECK_EQUAL(astJson["children"][0]["children"][0]["src"], "13:32:1");
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
