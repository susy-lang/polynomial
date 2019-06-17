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
 * Unit tests for the type system of Polynomial.
 */

#include <libpolynomial/Types.h>
#include <boost/test/unit_test.hpp>

using namespace std;

namespace dev
{
namespace polynomial
{
namespace test
{

BOOST_AUTO_TEST_SUITE(PolynomialTypes)

BOOST_AUTO_TEST_CASE(storage_layout_simple)
{
	MemberList members(MemberList::MemberMap({
		{string("first"), Type::fromElementaryTypeName("uint128")},
		{string("second"), Type::fromElementaryTypeName("uint120")},
		{string("wraps"), Type::fromElementaryTypeName("uint16")}
	}));
	BOOST_REQUIRE_EQUAL(u256(2), members.getStorageSize());
	BOOST_REQUIRE(members.getMemberStorageOffset("first") != nullptr);
	BOOST_REQUIRE(members.getMemberStorageOffset("second") != nullptr);
	BOOST_REQUIRE(members.getMemberStorageOffset("wraps") != nullptr);
	BOOST_CHECK(*members.getMemberStorageOffset("first") == make_pair(u256(0), unsigned(0)));
	BOOST_CHECK(*members.getMemberStorageOffset("second") == make_pair(u256(0), unsigned(16)));
	BOOST_CHECK(*members.getMemberStorageOffset("wraps") == make_pair(u256(1), unsigned(0)));
}

BOOST_AUTO_TEST_CASE(storage_layout_mapping)
{
	MemberList members(MemberList::MemberMap({
		{string("first"), Type::fromElementaryTypeName("uint128")},
		{string("second"), make_shared<MappingType>(
			Type::fromElementaryTypeName("uint8"),
			Type::fromElementaryTypeName("uint8")
		)},
		{string("third"), Type::fromElementaryTypeName("uint16")},
		{string("final"), make_shared<MappingType>(
			Type::fromElementaryTypeName("uint8"),
			Type::fromElementaryTypeName("uint8")
		)},
	}));
	BOOST_REQUIRE_EQUAL(u256(4), members.getStorageSize());
	BOOST_REQUIRE(members.getMemberStorageOffset("first") != nullptr);
	BOOST_REQUIRE(members.getMemberStorageOffset("second") != nullptr);
	BOOST_REQUIRE(members.getMemberStorageOffset("third") != nullptr);
	BOOST_REQUIRE(members.getMemberStorageOffset("final") != nullptr);
	BOOST_CHECK(*members.getMemberStorageOffset("first") == make_pair(u256(0), unsigned(0)));
	BOOST_CHECK(*members.getMemberStorageOffset("second") == make_pair(u256(1), unsigned(0)));
	BOOST_CHECK(*members.getMemberStorageOffset("third") == make_pair(u256(2), unsigned(0)));
	BOOST_CHECK(*members.getMemberStorageOffset("final") == make_pair(u256(3), unsigned(0)));
}

BOOST_AUTO_TEST_CASE(storage_layout_arrays)
{
	BOOST_CHECK(ArrayType(DataLocation::Storage, make_shared<FixedBytesType>(1), 32).getStorageSize() == 1);
	BOOST_CHECK(ArrayType(DataLocation::Storage, make_shared<FixedBytesType>(1), 33).getStorageSize() == 2);
	BOOST_CHECK(ArrayType(DataLocation::Storage, make_shared<FixedBytesType>(2), 31).getStorageSize() == 2);
	BOOST_CHECK(ArrayType(DataLocation::Storage, make_shared<FixedBytesType>(7), 8).getStorageSize() == 2);
	BOOST_CHECK(ArrayType(DataLocation::Storage, make_shared<FixedBytesType>(7), 9).getStorageSize() == 3);
	BOOST_CHECK(ArrayType(DataLocation::Storage, make_shared<FixedBytesType>(31), 9).getStorageSize() == 9);
	BOOST_CHECK(ArrayType(DataLocation::Storage, make_shared<FixedBytesType>(32), 9).getStorageSize() == 9);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
