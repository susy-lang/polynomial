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
 * Tests for high level features like import.
 */

#include <test/libpolynomial/ErrorCheck.h>
#include <test/Options.h>

#include <liblangutil/Exceptions.h>
#include <libpolynomial/interface/CompilerStack.h>

#include <boost/test/unit_test.hpp>

#include <string>

using namespace std;

namespace dev
{
namespace polynomial
{
namespace test
{

BOOST_AUTO_TEST_SUITE(PolynomialImports)

BOOST_AUTO_TEST_CASE(smoke_test)
{
	CompilerStack c;
	c.setSources({{"a", "contract C {} pragma polynomial >=0.0;"}});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(regular_import)
{
	CompilerStack c;
	c.setSources({
		{"a", "contract C {} pragma polynomial >=0.0;"},
		{"b", "import \"a\"; contract D is C {} pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(import_does_not_clutter_importee)
{
	CompilerStack c;
	c.setSources({
		{"a", "contract C { D d; } pragma polynomial >=0.0;"},
		{"b", "import \"a\"; contract D is C {} pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());
}

BOOST_AUTO_TEST_CASE(import_is_transitive)
{
	CompilerStack c;
	c.setSources({
		{"a", "contract C { } pragma polynomial >=0.0;"},
		{"b", "import \"a\"; pragma polynomial >=0.0;"},
		{"c", "import \"b\"; contract D is C {} pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(circular_import)
{
	CompilerStack c;
	c.setSources({
		{"a", "import \"b\"; contract C { D d; } pragma polynomial >=0.0;"},
		{"b", "import \"a\"; contract D { C c; } pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(relative_import)
{
	CompilerStack c;
	c.setSources({
		{"a", "import \"./dir/b\"; contract A is B {} pragma polynomial >=0.0;"},
		{"dir/b", "contract B {} pragma polynomial >=0.0;"},
		{"dir/c", "import \"../a\"; contract C is A {} pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(relative_import_multiplex)
{
	CompilerStack c;
	c.setSources({
		{"a", "contract A {} pragma polynomial >=0.0;"},
		{"dir/a/b/c", "import \"../../.././a\"; contract B is A {} pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(simple_alias)
{
	CompilerStack c;
	c.setSources({
		{"a", "contract A {} pragma polynomial >=0.0;"},
		{"dir/a/b/c", "import \"../../.././a\" as x; contract B is x.A { function() external { x.A r = x.A(20); } } pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(library_name_clash)
{
	CompilerStack c;
	c.setSources({
		{"a", "library A {} pragma polynomial >=0.0;"},
		{"b", "library A {} pragma polynomial >=0.0;"},
		{"c", "import {A} from \"./a\"; import {A} from \"./b\";"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());
}

BOOST_AUTO_TEST_CASE(library_name_clash_with_contract)
{
	CompilerStack c;
	c.setSources({
		{"a", "contract A {} pragma polynomial >=0.0;"},
		{"b", "library A {} pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(complex_import)
{
	CompilerStack c;
	c.setSources({
		{"a", "contract A {} contract B {} contract C { struct S { uint a; } } pragma polynomial >=0.0;"},
		{"b", "import \"a\" as x; import {B as b, C as c, C} from \"a\"; "
				"contract D is b { function f(c.S memory var1, x.C.S memory var2, C.S memory var3) internal {} } pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(name_clash_in_import_1)
{
	CompilerStack c;
	c.setSources({
		{"a", "contract A {} pragma polynomial >=0.0;"},
		{"b", "import \"a\"; contract A {} pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());
}

BOOST_AUTO_TEST_CASE(name_clash_in_import_2)
{
	CompilerStack c;
	c.setSources({
		{"a", "contract A {} pragma polynomial >=0.0;"},
		{"b", "import \"a\" as A; contract A {} pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());
}

BOOST_AUTO_TEST_CASE(name_clash_in_import_3)
{
	CompilerStack c;
	c.setSources({
		{"a", "contract A {} pragma polynomial >=0.0;"},
		{"b", "import {A as b} from \"a\"; contract b {} pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());
}

BOOST_AUTO_TEST_CASE(name_clash_in_import_4)
{
	CompilerStack c;
	c.setSources({
		{"a", "contract A {} pragma polynomial >=0.0;"},
		{"b", "import {A} from \"a\"; contract A {} pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());
}

BOOST_AUTO_TEST_CASE(name_clash_in_import_5)
{
	CompilerStack c;
	c.setSources({
		{"a", "contract A {} pragma polynomial >=0.0;"},
		{"b", "import {A} from \"a\"; contract B {} pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(remappings)
{
	CompilerStack c;
	c.setRemappings(vector<CompilerStack::Remapping>{{"", "s", "s_1.4.6"},{"", "t", "Tee"}});
	c.setSources({
		{"a", "import \"s/s.pol\"; contract A is S {} pragma polynomial >=0.0;"},
		{"b", "import \"t/tee.pol\"; contract A is Tee {} pragma polynomial >=0.0;"},
		{"s_1.4.6/s.pol", "contract S {} pragma polynomial >=0.0;"},
		{"Tee/tee.pol", "contract Tee {} pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(context_dependent_remappings)
{
	CompilerStack c;
	c.setRemappings(vector<CompilerStack::Remapping>{{"a", "s", "s_1.4.6"}, {"b", "s", "s_1.4.7"}});
	c.setSources({
		{"a/a.pol", "import \"s/s.pol\"; contract A is SSix {} pragma polynomial >=0.0;"},
		{"b/b.pol", "import \"s/s.pol\"; contract B is SSeven {} pragma polynomial >=0.0;"},
		{"s_1.4.6/s.pol", "contract SSix {} pragma polynomial >=0.0;"},
		{"s_1.4.7/s.pol", "contract SSeven {} pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(filename_with_period)
{
	CompilerStack c;
	c.setSources({
		{"a/a.pol", "import \".b.pol\"; contract A is B {} pragma polynomial >=0.0;"},
		{"a/.b.pol", "contract B {} pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());
}

BOOST_AUTO_TEST_CASE(context_dependent_remappings_ensure_default_and_module_preserved)
{
	CompilerStack c;
	c.setRemappings(vector<CompilerStack::Remapping>{
		{"", "foo", "vendor/foo_2.0.0"},
		{"vendor/bar", "foo", "vendor/foo_1.0.0"},
		{"", "bar", "vendor/bar"}
	});
	c.setSources({
		{"main.pol", "import \"foo/foo.pol\"; import {Bar} from \"bar/bar.pol\"; contract Main is Foo2, Bar {} pragma polynomial >=0.0;"},
		{"vendor/bar/bar.pol", "import \"foo/foo.pol\"; contract Bar {Foo1 foo;} pragma polynomial >=0.0;"},
		{"vendor/foo_1.0.0/foo.pol", "contract Foo1 {} pragma polynomial >=0.0;"},
		{"vendor/foo_2.0.0/foo.pol", "contract Foo2 {} pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(context_dependent_remappings_order_independent_1)
{
	CompilerStack c;
	c.setRemappings(vector<CompilerStack::Remapping>{{"a", "x/y/z", "d"}, {"a/b", "x", "e"}});
	c.setSources({
		{"a/main.pol", "import \"x/y/z/z.pol\"; contract Main is D {} pragma polynomial >=0.0;"},
		{"a/b/main.pol", "import \"x/y/z/z.pol\"; contract Main is E {} pragma polynomial >=0.0;"},
		{"d/z.pol", "contract D {} pragma polynomial >=0.0;"},
		{"e/y/z/z.pol", "contract E {} pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(context_dependent_remappings_order_independent_2)
{
	CompilerStack c;
	c.setRemappings(vector<CompilerStack::Remapping>{{"a/b", "x", "e"}, {"a", "x/y/z", "d"}});
	c.setSources({
		{"a/main.pol", "import \"x/y/z/z.pol\"; contract Main is D {} pragma polynomial >=0.0;"},
		{"a/b/main.pol", "import \"x/y/z/z.pol\"; contract Main is E {} pragma polynomial >=0.0;"},
		{"d/z.pol", "contract D {} pragma polynomial >=0.0;"},
		{"e/y/z/z.pol", "contract E {} pragma polynomial >=0.0;"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(shadowing_via_import)
{
	CompilerStack c;
	c.setSources({
		{"a", "library A {} pragma polynomial >=0.0;"},
		{"b", "library A {} pragma polynomial >=0.0;"},
		{"c", "import {A} from \"./a\"; import {A} from \"./b\";"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());
}

BOOST_AUTO_TEST_CASE(shadowing_builtins_with_imports)
{
	CompilerStack c;
	c.setSources({
		{"B.pol", "contract X {} pragma polynomial >=0.0;"},
		{"b", R"(
		pragma polynomial >=0.0;
		import * as msg from "B.pol";
		contract C {
		})"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
	size_t errorCount = 0;
	for (auto const& e: c.errors())
	{
		string const* msg = e->comment();
		BOOST_REQUIRE(msg);
		if (msg->find("pre-release") != string::npos)
			continue;
		BOOST_CHECK(
			msg->find("shadows a builtin symbol") != string::npos
		);
		errorCount++;
	}
	BOOST_CHECK_EQUAL(errorCount, 1);
}

BOOST_AUTO_TEST_CASE(shadowing_builtins_with_multiple_imports)
{
	CompilerStack c;
	c.setSources({
		{"B.pol", "contract msg {} contract block{} pragma polynomial >=0.0;"},
		{"b", R"(
		pragma polynomial >=0.0;
		import {msg, block} from "B.pol";
		contract C {
		})"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
	auto numErrors = c.errors().size();
	// Sometimes we get the prerelease warning, sometimes not.
	BOOST_CHECK(4 <= numErrors && numErrors <= 5);
	for (auto const& e: c.errors())
	{
		string const* msg = e->comment();
		BOOST_REQUIRE(msg);
		BOOST_CHECK(
			msg->find("pre-release") != string::npos ||
			msg->find("shadows a builtin symbol") != string::npos
		);
	}
}

BOOST_AUTO_TEST_CASE(shadowing_builtins_with_alias)
{
	CompilerStack c;
	c.setSources({
		{"B.pol", "contract C {} pragma polynomial >=0.0;"},
		{"b", R"(
		pragma polynomial >=0.0;
		import {C as msg} from "B.pol";)"}
	});
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
	auto numErrors = c.errors().size();
	// Sometimes we get the prerelease warning, sometimes not.
	BOOST_CHECK(1 <= numErrors && numErrors <= 2);
	for (auto const& e: c.errors())
	{
		string const* msg = e->comment();
		BOOST_REQUIRE(msg);
		BOOST_CHECK(
			msg->find("pre-release") != string::npos ||
			msg->find("shadows a builtin symbol") != string::npos
		);
	}
}

BOOST_AUTO_TEST_CASE(inheritance_abi_encoder_mismatch_1)
{
	CompilerStack c;
	c.setSources({
	{"A.pol", R"(
		pragma polynomial >=0.0;
		pragma experimental ABIEncoderV2;

		contract A
		{
			struct S { uint a; }
			S public s;
			function f(S memory _s) returns (S memory,S memory) { }
		}
	)"},
	{"B.pol", R"(
		pragma polynomial >=0.0;
		pragma experimental ABIEncoderV2;

		import "./A.pol";
		contract B is A { }
	)"},
	{"C.pol", R"(
		pragma polynomial >=0.0;

		import "./B.pol";
		contract C is B { }
	)"}
	});

	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());

	int typeErrors = 0;

	// Sometimes we get the prerelease warning, sometimes not.
	for (auto const& e: c.errors())
	{
		if (e->type() != langutil::Error::Type::TypeError)
			continue;

		typeErrors++;

		string const* msg = e->comment();
		BOOST_REQUIRE(msg);
		BOOST_CHECK_EQUAL(*msg, std::string("Contract \"C\" does not use the new experimental ABI encoder but wants to inherit from a contract which uses types that require it. Use \"pragma experimental ABIEncoderV2;\" for the inheriting contract as well to enable the feature."));
	}
	BOOST_CHECK_EQUAL(typeErrors, 1);
}

BOOST_AUTO_TEST_CASE(inheritance_abi_encoder_mismatch_2)
{
	CompilerStack c;
	c.setSources({
	{"A.pol", R"(
		pragma polynomial >=0.0;
		pragma experimental ABIEncoderV2;

		contract A
		{
			struct S { uint a; }
			S public s;
			function f(S memory _s) returns (S memory,S memory) { }
		}
	)"},
	{"B.pol", R"(
		pragma polynomial >=0.0;

		import "./A.pol";
		contract B is A { }
	)"},
	{"C.pol", R"(
		pragma polynomial >=0.0;

		import "./B.pol";
		contract C is B { }
	)"}
	});

	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());

	int typeErrors = 0;

	// Sometimes we get the prerelease warning, sometimes not.
	for (auto const& e: c.errors())
	{
		if (e->type() != langutil::Error::Type::TypeError)
			continue;

		typeErrors++;

		string const* msg = e->comment();
		BOOST_REQUIRE(msg);
		BOOST_CHECK_EQUAL(*msg, std::string("Contract \"B\" does not use the new experimental ABI encoder but wants to inherit from a contract which uses types that require it. Use \"pragma experimental ABIEncoderV2;\" for the inheriting contract as well to enable the feature."));
	}
	BOOST_CHECK_EQUAL(typeErrors, 1);
}

BOOST_AUTO_TEST_CASE(inheritance_abi_encoder_match)
{
	CompilerStack c;
	c.setSources({
	{"A.pol", R"(
		pragma polynomial >=0.0;
		pragma experimental ABIEncoderV2;

		contract A
		{
			struct S { uint a; }
			S public s;
			function f(S memory _s) public returns (S memory,S memory) { }
		}
	)"},
	{"B.pol", R"(
		pragma polynomial >=0.0;
		pragma experimental ABIEncoderV2;

		import "./A.pol";
		contract B is A { }
	)"},
	{"C.pol", R"(
		pragma polynomial >=0.0;
		pragma experimental ABIEncoderV2;

		import "./B.pol";
		contract C is B { }
	)"}
	});

	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());

	int typeErrors = 0;

	// Sometimes we get the prerelease warning, sometimes not.
	for (auto const& e: c.errors())
	{
		if (e->type() != langutil::Error::Type::TypeError)
			continue;

		typeErrors++;
	}

	BOOST_CHECK_EQUAL(typeErrors, 0);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
