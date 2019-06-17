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
 * @author Lefteris Karapetsas <lefteris@sofdev.com>
 * @date 2015
 * Unit tests for Assembly Items from svmasm/Assembly.h
 */

#include <string>
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <libsvmasm/SourceLocation.h>
#include <libsvmasm/Assembly.h>
#include <libpolynomial/parsing/Scanner.h>
#include <libpolynomial/parsing/Parser.h>
#include <libpolynomial/analysis/NameAndTypeResolver.h>
#include <libpolynomial/codegen/Compiler.h>
#include <libpolynomial/ast/AST.h>
#include <libpolynomial/analysis/TypeChecker.h>

using namespace std;
using namespace dev::sof;

namespace dev
{
namespace polynomial
{
namespace test
{

namespace
{

sof::AssemblyItems compileContract(const string& _sourceCode)
{
	ErrorList errors;
	Parser parser(errors);
	ASTPointer<SourceUnit> sourceUnit;
	BOOST_REQUIRE_NO_THROW(sourceUnit = parser.parse(make_shared<Scanner>(CharStream(_sourceCode))));
	BOOST_CHECK(!!sourceUnit);

	map<ASTNode const*, shared_ptr<DeclarationContainer>> scopes;
	NameAndTypeResolver resolver({}, scopes, errors);
	polAssert(Error::containsOnlyWarnings(errors), "");
	resolver.registerDeclarations(*sourceUnit);
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			BOOST_REQUIRE_NO_THROW(resolver.resolveNamesAndTypes(*contract));
			if (!Error::containsOnlyWarnings(errors))
				return AssemblyItems();
		}
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			TypeChecker checker(errors);
			BOOST_REQUIRE_NO_THROW(checker.checkTypeRequirements(*contract));
			if (!Error::containsOnlyWarnings(errors))
				return AssemblyItems();
		}
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			Compiler compiler;
			compiler.compileContract(*contract, map<ContractDefinition const*, Assembly const*>{}, bytes());

			return compiler.runtimeAssemblyItems();
		}
	BOOST_FAIL("No contract found in source.");
	return AssemblyItems();
}

void checkAssemblyLocations(AssemblyItems const& _items, vector<SourceLocation> const& _locations)
{
	BOOST_CHECK_EQUAL(_items.size(), _locations.size());
	for (size_t i = 0; i < min(_items.size(), _locations.size()); ++i)
	{
		BOOST_CHECK_MESSAGE(
			_items[i].location() == _locations[i],
			"Location mismatch for assembly item " + to_string(i) + ". Found: " +
					(_items[i].location().sourceName ? *_items[i].location().sourceName + ":" : "(null source name)") +
					to_string(_items[i].location().start) + "-" +
					to_string(_items[i].location().end) + ", expected: " +
					(_locations[i].sourceName ? *_locations[i].sourceName + ":" : "(null source name)") +
					to_string(_locations[i].start) + "-" +
					to_string(_locations[i].end));
	}
}

} // end anonymous namespace

BOOST_AUTO_TEST_SUITE(Assembly)

BOOST_AUTO_TEST_CASE(location_test)
{
	char const* sourceCode = R"(
	contract test {
		function f() returns (uint256 a) {
			return 16;
		}
	}
	)";
	shared_ptr<string const> n = make_shared<string>("");
	AssemblyItems items = compileContract(sourceCode);
	vector<SourceLocation> locations =
		vector<SourceLocation>(17, SourceLocation(2, 75, n)) +
		vector<SourceLocation>(30, SourceLocation(20, 72, n)) +
		vector<SourceLocation>{SourceLocation(42, 51, n), SourceLocation(65, 67, n)} +
		vector<SourceLocation>(2, SourceLocation(58, 67, n)) +
		vector<SourceLocation>(3, SourceLocation(20, 72, n));
	checkAssemblyLocations(items, locations);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
