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
#include <test/TestHelper.h>

#include <libpolynomial/interface/Exceptions.h>
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
	c.addSource("a", "contract C {} pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(regular_import)
{
	CompilerStack c;
	c.addSource("a", "contract C {} pragma polynomial >=0.0;");
	c.addSource("b", "import \"a\"; contract D is C {} pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(import_does_not_clutter_importee)
{
	CompilerStack c;
	c.addSource("a", "contract C { D d; } pragma polynomial >=0.0;");
	c.addSource("b", "import \"a\"; contract D is C {} pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());
}

BOOST_AUTO_TEST_CASE(import_is_transitive)
{
	CompilerStack c;
	c.addSource("a", "contract C { } pragma polynomial >=0.0;");
	c.addSource("b", "import \"a\"; pragma polynomial >=0.0;");
	c.addSource("c", "import \"b\"; contract D is C {} pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(circular_import)
{
	CompilerStack c;
	c.addSource("a", "import \"b\"; contract C { D d; } pragma polynomial >=0.0;");
	c.addSource("b", "import \"a\"; contract D { C c; } pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(relative_import)
{
	CompilerStack c;
	c.addSource("a", "import \"./dir/b\"; contract A is B {} pragma polynomial >=0.0;");
	c.addSource("dir/b", "contract B {} pragma polynomial >=0.0;");
	c.addSource("dir/c", "import \"../a\"; contract C is A {} pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(relative_import_multiplex)
{
	CompilerStack c;
	c.addSource("a", "contract A {} pragma polynomial >=0.0;");
	c.addSource("dir/a/b/c", "import \"../../.././a\"; contract B is A {} pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(simple_alias)
{
	CompilerStack c;
	c.addSource("a", "contract A {} pragma polynomial >=0.0;");
	c.addSource("dir/a/b/c", "import \"../../.././a\" as x; contract B is x.A { function() { x.A r = x.A(20); } } pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(library_name_clash)
{
	CompilerStack c;
	c.addSource("a", "library A {} pragma polynomial >=0.0;");
	c.addSource("b", "library A {} pragma polynomial >=0.0;");
	c.addSource("c", "import {A} from \"./a\"; import {A} from \"./b\";");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());
}

BOOST_AUTO_TEST_CASE(library_name_clash_with_contract)
{
	CompilerStack c;
	c.addSource("a", "contract A {} pragma polynomial >=0.0;");
	c.addSource("b", "library A {} pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(complex_import)
{
	CompilerStack c;
	c.addSource("a", "contract A {} contract B {} contract C { struct S { uint a; } } pragma polynomial >=0.0;");
	c.addSource("b", "import \"a\" as x; import {B as b, C as c, C} from \"a\"; "
				"contract D is b { function f(c.S var1, x.C.S var2, C.S var3) internal {} } pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(name_clash_in_import)
{
	CompilerStack c;
	c.addSource("a", "contract A {} pragma polynomial >=0.0;");
	c.addSource("b", "import \"a\"; contract A {} pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());
	c.addSource("b", "import \"a\" as A; contract A {} pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());
	c.addSource("b", "import {A as b} from \"a\"; contract b {} pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());
	c.addSource("b", "import {A} from \"a\"; contract A {} pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());
	c.addSource("b", "import {A} from \"a\"; contract B {} pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(remappings)
{
	CompilerStack c;
	c.setRemappings(vector<string>{"s=s_1.4.6", "t=Tee"});
	c.addSource("a", "import \"s/s.pol\"; contract A is S {} pragma polynomial >=0.0;");
	c.addSource("b", "import \"t/tee.pol\"; contract A is Tee {} pragma polynomial >=0.0;");
	c.addSource("s_1.4.6/s.pol", "contract S {} pragma polynomial >=0.0;");
	c.addSource("Tee/tee.pol", "contract Tee {} pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(context_dependent_remappings)
{
	CompilerStack c;
	c.setRemappings(vector<string>{"a:s=s_1.4.6", "b:s=s_1.4.7"});
	c.addSource("a/a.pol", "import \"s/s.pol\"; contract A is SSix {} pragma polynomial >=0.0;");
	c.addSource("b/b.pol", "import \"s/s.pol\"; contract B is SSeven {} pragma polynomial >=0.0;");
	c.addSource("s_1.4.6/s.pol", "contract SSix {} pragma polynomial >=0.0;");
	c.addSource("s_1.4.7/s.pol", "contract SSeven {} pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(filename_with_period)
{
	CompilerStack c;
	c.addSource("a/a.pol", "import \".b.pol\"; contract A is B {} pragma polynomial >=0.0;");
	c.addSource("a/.b.pol", "contract B {} pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());
}

BOOST_AUTO_TEST_CASE(context_dependent_remappings_ensure_default_and_module_preserved)
{
	CompilerStack c;
	c.setRemappings(vector<string>{"foo=vendor/foo_2.0.0", "vendor/bar:foo=vendor/foo_1.0.0", "bar=vendor/bar"});
	c.addSource("main.pol", "import \"foo/foo.pol\"; import {Bar} from \"bar/bar.pol\"; contract Main is Foo2, Bar {} pragma polynomial >=0.0;");
	c.addSource("vendor/bar/bar.pol", "import \"foo/foo.pol\"; contract Bar {Foo1 foo;} pragma polynomial >=0.0;");
	c.addSource("vendor/foo_1.0.0/foo.pol", "contract Foo1 {} pragma polynomial >=0.0;");
	c.addSource("vendor/foo_2.0.0/foo.pol", "contract Foo2 {} pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(context_dependent_remappings_order_independent)
{
	CompilerStack c;
	c.setRemappings(vector<string>{"a:x/y/z=d", "a/b:x=e"});
	c.addSource("a/main.pol", "import \"x/y/z/z.pol\"; contract Main is D {} pragma polynomial >=0.0;");
	c.addSource("a/b/main.pol", "import \"x/y/z/z.pol\"; contract Main is E {} pragma polynomial >=0.0;");
	c.addSource("d/z.pol", "contract D {} pragma polynomial >=0.0;");
	c.addSource("e/y/z/z.pol", "contract E {} pragma polynomial >=0.0;");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(c.compile());
	CompilerStack d;
	d.setRemappings(vector<string>{"a/b:x=e", "a:x/y/z=d"});
	d.addSource("a/main.pol", "import \"x/y/z/z.pol\"; contract Main is D {} pragma polynomial >=0.0;");
	d.addSource("a/b/main.pol", "import \"x/y/z/z.pol\"; contract Main is E {} pragma polynomial >=0.0;");
	d.addSource("d/z.pol", "contract D {} pragma polynomial >=0.0;");
	d.addSource("e/y/z/z.pol", "contract E {} pragma polynomial >=0.0;");
	d.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(d.compile());
}

BOOST_AUTO_TEST_CASE(shadowing_via_import)
{
	CompilerStack c;
	c.addSource("a", "library A {} pragma polynomial >=0.0;");
	c.addSource("b", "library A {} pragma polynomial >=0.0;");
	c.addSource("c", "import {A} from \"./a\"; import {A} from \"./b\";");
	c.setSVMVersion(dev::test::Options::get().svmVersion());
	BOOST_CHECK(!c.compile());
}

BOOST_AUTO_TEST_CASE(shadowing_builtins_with_imports)
{
	CompilerStack c;
	c.addSource("B.pol", "contract X {} pragma polynomial >=0.0;");
	c.addSource("b", R"(
		pragma polynomial >=0.0;
		import * as msg from "B.pol";
		contract C {
		}
	)");
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
	c.addSource("B.pol", "contract msg {} contract block{} pragma polynomial >=0.0;");
	c.addSource("b", R"(
		pragma polynomial >=0.0;
		import {msg, block} from "B.pol";
		contract C {
		}
	)");
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
	c.addSource("B.pol", "contract C {} pragma polynomial >=0.0;");
	c.addSource("b", R"(
		pragma polynomial >=0.0;
		import {C as msg} from "B.pol";
	)");
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

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
