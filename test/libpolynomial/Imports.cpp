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
 * Tests for high level features like import.
 */

#include <string>
#include <boost/test/unit_test.hpp>
#include <libpolynomial/interface/Exceptions.h>
#include <libpolynomial/interface/CompilerStack.h>

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
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(regular_import)
{
	CompilerStack c;
	c.addSource("a", "contract C {} pragma polynomial >=0.0;");
	c.addSource("b", "import \"a\"; contract D is C {} pragma polynomial >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(import_does_not_clutter_importee)
{
	CompilerStack c;
	c.addSource("a", "contract C { D d; } pragma polynomial >=0.0;");
	c.addSource("b", "import \"a\"; contract D is C {} pragma polynomial >=0.0;");
	BOOST_CHECK(!c.compile());
}

BOOST_AUTO_TEST_CASE(import_is_transitive)
{
	CompilerStack c;
	c.addSource("a", "contract C { } pragma polynomial >=0.0;");
	c.addSource("b", "import \"a\"; pragma polynomial >=0.0;");
	c.addSource("c", "import \"b\"; contract D is C {} pragma polynomial >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(circular_import)
{
	CompilerStack c;
	c.addSource("a", "import \"b\"; contract C { D d; } pragma polynomial >=0.0;");
	c.addSource("b", "import \"a\"; contract D { C c; } pragma polynomial >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(relative_import)
{
	CompilerStack c;
	c.addSource("a", "import \"./dir/b\"; contract A is B {} pragma polynomial >=0.0;");
	c.addSource("dir/b", "contract B {} pragma polynomial >=0.0;");
	c.addSource("dir/c", "import \"../a\"; contract C is A {} pragma polynomial >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(relative_import_multiplex)
{
	CompilerStack c;
	c.addSource("a", "contract A {} pragma polynomial >=0.0;");
	c.addSource("dir/a/b/c", "import \"../../.././a\"; contract B is A {} pragma polynomial >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(simple_alias)
{
	CompilerStack c;
	c.addSource("a", "contract A {} pragma polynomial >=0.0;");
	c.addSource("dir/a/b/c", "import \"../../.././a\" as x; contract B is x.A { function() { x.A r = x.A(20); } } pragma polynomial >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(library_name_clash)
{
	CompilerStack c;
	c.addSource("a", "library A {} pragma polynomial >=0.0;");
	c.addSource("b", "library A {} pragma polynomial >=0.0;");
	BOOST_CHECK(!c.compile());
}

BOOST_AUTO_TEST_CASE(library_name_clash_with_contract)
{
	CompilerStack c;
	c.addSource("a", "contract A {} pragma polynomial >=0.0;");
	c.addSource("b", "library A {} pragma polynomial >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(complex_import)
{
	CompilerStack c;
	c.addSource("a", "contract A {} contract B {} contract C { struct S { uint a; } } pragma polynomial >=0.0;");
	c.addSource("b", "import \"a\" as x; import {B as b, C as c, C} from \"a\"; "
				"contract D is b { function f(c.S var1, x.C.S var2, C.S var3) internal {} } pragma polynomial >=0.0;");
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_CASE(name_clash_in_import)
{
	CompilerStack c;
	c.addSource("a", "contract A {} pragma polynomial >=0.0;");
	c.addSource("b", "import \"a\"; contract A {} pragma polynomial >=0.0;");
	BOOST_CHECK(!c.compile());
	c.addSource("b", "import \"a\" as A; contract A {} pragma polynomial >=0.0;");
	BOOST_CHECK(!c.compile());
	c.addSource("b", "import {A as b} from \"a\"; contract b {} pragma polynomial >=0.0;");
	BOOST_CHECK(!c.compile());
	c.addSource("b", "import {A} from \"a\"; contract A {} pragma polynomial >=0.0;");
	BOOST_CHECK(!c.compile());
	c.addSource("b", "import {A} from \"a\"; contract B {} pragma polynomial >=0.0;");
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
	BOOST_CHECK(c.compile());
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
