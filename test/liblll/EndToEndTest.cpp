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
 * @author Alex Beregszaszi
 * @date 2016
 * End to end tests for LLL.
 */

#include <string>
#include <memory>
#include <boost/test/unit_test.hpp>
#include <test/liblll/ExecutionFramework.h>

using namespace std;

namespace dev
{
namespace lll
{
namespace test
{

BOOST_FIXTURE_TEST_SUITE(LLLEndToEndTest, LLLExecutionFramework)

BOOST_AUTO_TEST_CASE(smoke_test)
{
        char const* sourceCode = "(returnlll { (return \"test\") })";
        compileAndRun(sourceCode);
        BOOST_CHECK(callFallback() == encodeArgs(string("test", 4)));
}

BOOST_AUTO_TEST_CASE(bare_panic)
{
	char const* sourceCode = "(panic)";
	compileAndRunWithoutCheck(sourceCode);
	BOOST_REQUIRE(m_output.empty());
}

BOOST_AUTO_TEST_CASE(panic)
{
	char const* sourceCode = "{ (panic) }";
	compileAndRunWithoutCheck(sourceCode);
	BOOST_REQUIRE(m_output.empty());
}

BOOST_AUTO_TEST_CASE(macro_zeroarg)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(def 'zeroarg () (seq (mstore 0 0x1234) (return 0 32)))
				(zeroarg)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(0x1234)));
}

BOOST_AUTO_TEST_CASE(macros)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(def 'x 1)
				(def 'y () { (def 'x (+ x 2)) })
				(y)
				(return x)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(3)));
}

BOOST_AUTO_TEST_CASE(variables)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(set 'x 1)
				(set 'y 2)
				;; this should equal to 3
				(set 'z (add (get 'x) (get 'y)))
				;; overwriting it here
				(set 'y 4)
				;; each variable has a 32 byte slot, starting from memory location 0x80
				;; variable addresses can also be retrieved by x or (ref 'x)
				(set 'k (add (add (ref 'x) (ref 'y)) z))
				(return (add (add (get 'x) (add (get 'y) (get 'z))) (get 'k)))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(488)));
}

BOOST_AUTO_TEST_CASE(when)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(when (= (calldatasize) 0) (return 1))
				(when (!= (calldatasize) 0) (return 2))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("test()") == encodeArgs(u256(2)));
	BOOST_CHECK(callFallback() == toBigEndian(u256(1)));
}

BOOST_AUTO_TEST_CASE(unless)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(unless (!= (calldatasize) 0) (return 1))
				(unless (= (calldatasize) 0) (return 2))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("test()") == encodeArgs(u256(2)));
	BOOST_CHECK(callFallback() == toBigEndian(u256(1)));
}

BOOST_AUTO_TEST_CASE(conditional_literal)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(return (if (= (calldatasize) 0) 1 2))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("test()") == encodeArgs(u256(2)));
	BOOST_CHECK(callFallback() == toBigEndian(u256(1)));
}

BOOST_AUTO_TEST_CASE(conditional)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(if (= (calldatasize) 0) (return 1) (return 2))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("test()") == encodeArgs(u256(2)));
	BOOST_CHECK(callFallback() == toBigEndian(u256(1)));
}

BOOST_AUTO_TEST_CASE(conditional_seq)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(return (if (= (calldatasize) 0) { 0 2 1 } { 0 1 2 }))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("test()") == encodeArgs(u256(2)));
	BOOST_CHECK(callFallback() == toBigEndian(u256(1)));
}

BOOST_AUTO_TEST_CASE(exp_operator_const)
{
	char const* sourceCode = R"(
		(returnlll
			(return (exp 2 3)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == toBigEndian(u256(8)));
}

BOOST_AUTO_TEST_CASE(exp_operator_const_signed)
{
	char const* sourceCode = R"(
		(returnlll
			(return (exp (- 0 2) 3)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == toBigEndian(u256(-8)));
}

BOOST_AUTO_TEST_CASE(exp_operator_on_range)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(when (= (div (calldataload 0x00) (exp 2 224)) 0xb3de648b)
					(return (exp 2 (calldataload 0x04))))
				(jump 0x02)))
	)";
	compileAndRun(sourceCode);
	testContractAgainstCppOnRange("f(uint256)", [](u256 const& a) -> u256 { return u256(1 << a.convert_to<int>()); }, 0, 16);
}

BOOST_AUTO_TEST_CASE(constructor_argument_internal_numeric)
{
	char const* sourceCode = R"(
		(seq
			(sstore 0x00 65535)
			(returnlll
				(return @@0x00)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(65535)));
}

BOOST_AUTO_TEST_CASE(constructor_argument_internal_string)
{
	char const* sourceCode = R"(
		(seq
			(sstore 0x00 "test")
			(returnlll
				(return @@0x00)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs("test"));
}

BOOST_AUTO_TEST_CASE(constructor_arguments_external)
{
	char const* sourceCode = R"(
		(seq
			(codecopy 0x00 (bytecodesize) 64)
			(sstore 0x00 @0x00)
			(sstore 0x01 @0x20)
			(returnlll
				(seq
					(when (= (div (calldataload 0x00) (exp 2 224)) 0xf2c9ecd8)
						(return @@0x00))
					(when (= (div (calldataload 0x00) (exp 2 224)) 0x89ea642f)
						(return @@0x01)))))
	)";
	compileAndRun(sourceCode, 0, "", encodeArgs(u256(65535), "test"));
	BOOST_CHECK(callContractFunction("getNumber()") == encodeArgs(u256(65535)));
	BOOST_CHECK(callContractFunction("getString()") == encodeArgs("test"));
}

BOOST_AUTO_TEST_CASE(fallback_and_invalid_function)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(when (= (div (calldataload 0x00) (exp 2 224)) 0xab5ed150)
					(return "one"))
				(when (= (div (calldataload 0x00) (exp 2 224)) 0xee784123)
					(return "two"))
				(return "three")))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callContractFunction("getOne()") == encodeArgs("one"));
	BOOST_CHECK(callContractFunction("getTwo()") == encodeArgs("two"));
	BOOST_CHECK(callContractFunction("invalidFunction()") == encodeArgs("three"));
	BOOST_CHECK(callFallback() == encodeArgs("three"));
}

BOOST_AUTO_TEST_CASE(lit_string)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(lit 0x00 "abcdef")
				(return 0x00 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(string("abcdef")));
}

BOOST_AUTO_TEST_CASE(arithmetic)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(mstore8 0x00 (+ 160 22))
				(mstore8 0x01 (- 223 41))
				(mstore8 0x02 (* 33 2))
				(mstore8 0x03 (/ 10 2))
				(mstore8 0x04 (% 67 2))
				(mstore8 0x05 (& 15 8))
				(mstore8 0x06 (| 18 8))
				(mstore8 0x07 (^ 26 6))
				(return 0x00 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("b6b6420501081a1c000000000000000000000000000000000000000000000000")));
}

BOOST_AUTO_TEST_CASE(binary)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(mstore8 0x00 (< 53 87))
				(mstore8 0x01 (< 73 42))
				(mstore8 0x02 (<= 37 94))
				(mstore8 0x03 (<= 37 37))
				(mstore8 0x04 (<= 183 34))
				(mstore8 0x05 (S< (- 0 53) 87))
				(mstore8 0x06 (S< 73 (- 0 42)))
				(mstore8 0x07 (S<= (- 0 37) 94))
				(mstore8 0x08 (S<= (- 0 37) (- 0 37)))
				(mstore8 0x09 (S<= 183 (- 0 34)))
				(mstore8 0x0a (> 73 42))
				(mstore8 0x0b (> 53 87))
				(mstore8 0x0c (>= 94 37))
				(mstore8 0x0d (>= 94 94))
				(mstore8 0x0e (>= 34 183))
				(mstore8 0x0f (S> 73 (- 0 42)))
				(mstore8 0x10 (S> (- 0 53) 87))
				(mstore8 0x11 (S>= 94 (- 0 37)))
				(mstore8 0x12 (S>= (- 0 94) (- 0 94)))
				(mstore8 0x13 (S>= (- 0 34) 183))
				(mstore8 0x14 (= 53 53))
				(mstore8 0x15 (= 73 42))
				(mstore8 0x16 (!= 37 94))
				(mstore8 0x17 (!= 37 37))
				(return 0x00 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("0100010100010001010001000101000100010100010001000000000000000000")));
}

BOOST_AUTO_TEST_CASE(unary)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(mstore8 0x00 (! (< 53 87)))
				(mstore8 0x01 (! (>= 42 73)))
				(mstore8 0x02 (~ 0x7f))
				(mstore8 0x03 (~ 0xaa))
				(return 0x00 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("0001805500000000000000000000000000000000000000000000000000000000")));
}

BOOST_AUTO_TEST_CASE(assembly_mload_mstore)
{
	char const* sourceCode = R"(
		(returnlll
			(asm
				0x07 0x00 mstore
				"abcdef" 0x20 mstore
				0x00 mload 0x40 mstore
				0x20 mload 0x60 mstore
				0x40 0x40 return))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(7), string("abcdef")));
}

BOOST_AUTO_TEST_CASE(assembly_sload_sstore)
{
	char const* sourceCode = R"(
		(returnlll
			(asm
				0x07 0x00 sstore
				"abcdef" 0x01 sstore
				0x00 sload 0x00 mstore
				0x01 sload 0x20 mstore
				0x40 0x00 return))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(7), string("abcdef")));
}

BOOST_AUTO_TEST_CASE(assembly_codecopy)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(lit 0x00 "abcdef")
				(asm
					0x06 6 codesize sub 0x20 codecopy
					0x20 0x20 return)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(string("abcdef")));
}


BOOST_AUTO_TEST_CASE(keccak256_32bytes)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(mstore 0x00 0x01)
				(return (keccak256 0x00 0x20))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("b10e2d527612073b26eecdfd717e6a320cf44b4afac2b0732d9fcbe2b7fa0cf6")));
}

// The following tests are for the built-in macros.
// Note that panic, returnlll and return_one_arg are well covered above.

BOOST_AUTO_TEST_CASE(allgas)
{
	char const* sourceCode = R"(
		(returnlll
			(return (- (gas) allgas)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(16))); // == 21 - SUB - GAS
}

BOOST_AUTO_TEST_CASE(send_two_args)
{
	char const* sourceCode = R"(
		(returnlll
			(send 0xdead 42))
	)";
	compileAndRun(sourceCode);
	callFallbackWithValue(42);
	BOOST_CHECK(balanceAt(Address(0xdead)) == 42);
}

BOOST_AUTO_TEST_CASE(send_three_args)
{
	char const* sourceCode = R"(
		(returnlll
			(send allgas 0xdead 42))
	)";
	compileAndRun(sourceCode);
	callFallbackWithValue(42);
	BOOST_CHECK(balanceAt(Address(0xdead)) == 42);
}

BOOST_AUTO_TEST_CASE(msg_six_args)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(when (= 0 (calldatasize))
					(seq
						(mstore 0x40 1)
						(def 'outsize 0x20)
						(return (msg 1000 (address) 42 0x40 0x20 outsize) outsize)))
				(when (= 1 (calldataload 0x00))
					(return (callvalue)))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallbackWithValue(42) == encodeArgs(u256(42)));
}

BOOST_AUTO_TEST_CASE(msg_five_args)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(when (= 0 (calldatasize))
					(seq
						(mstore 0x20 1)
						(mstore 0x40 2)
						(return (msg 1000 (address) 42 0x20 0x40))))
				(when (= 3 (+ (calldataload 0x00) (calldataload 0x20)))
					(return (callvalue)))))

	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallbackWithValue(42) == encodeArgs(u256(42)));
}

BOOST_AUTO_TEST_CASE(msg_four_args)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(when (= 0 (calldatasize))
					(return (msg 1000 (address) 42 0xff)))
				(return (callvalue))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallbackWithValue(42) == encodeArgs(u256(42)));
}

BOOST_AUTO_TEST_CASE(msg_three_args)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(when (= 0 (calldatasize))
					(return (msg (address) 42 0xff)))
				(return (callvalue))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallbackWithValue(42) == encodeArgs(u256(42)));
}

BOOST_AUTO_TEST_CASE(msg_two_args)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(when (= 0 (calldatasize))
					(return (msg (address) 0xff)))
				(return 42)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(42)));
}

BOOST_AUTO_TEST_CASE(create_one_arg)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(call allgas
					(create (returnlll (return 42)))
					0 0 0 0x00 0x20)
				(return 0x00 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(42)));
}

BOOST_AUTO_TEST_CASE(create_two_args)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(call allgas
					(create 42 (returnlll (return (balance (address)))))
					0 0 0 0x00 0x20)
				(return 0x00 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallbackWithValue(42) == encodeArgs(u256(42)));
}

BOOST_AUTO_TEST_CASE(sha3_two_args)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(mstore 0x00 0x01)
				(return (sha3 0x00 0x20))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("b10e2d527612073b26eecdfd717e6a320cf44b4afac2b0732d9fcbe2b7fa0cf6")));
}

BOOST_AUTO_TEST_CASE(sha3_one_arg)
{
	char const* sourceCode = R"(
		(returnlll
			(return (sha3 0x01)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("b10e2d527612073b26eecdfd717e6a320cf44b4afac2b0732d9fcbe2b7fa0cf6")));
}

BOOST_AUTO_TEST_CASE(sha3pair)
{
	char const* sourceCode = R"(
		(returnlll
			(return (sha3pair 0x01 0x02)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("0xe90b7bceb6e7df5418fb78d8ee546e97c83a08bbccc01a0644d599ccd2a7c2e0")));
}

BOOST_AUTO_TEST_CASE(sha3trip)
{
	char const* sourceCode = R"(
		(returnlll
			(return (sha3trip 0x01 0x02 0x03)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("0x6e0c627900b24bd432fe7b1f713f1b0744091a646a9fe4a65a18dfed21f2949c")));
}

BOOST_AUTO_TEST_CASE(makeperm) // Covers makeperm (implicit), permcount and perm
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(perm 'x) (x (+ 1 x))
				(perm 'y) (y (+ 10 y))
				(when (= 2 permcount)
					(return (+ x y)))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(11)));
}

BOOST_AUTO_TEST_CASE(ecrecover)
{
	char const* sourceCode = R"(
		(returnlll
			(return
				(ecrecover
					; Hash of 'hello world'
					0x47173285a8d7341e5e972fc677286384f802f8ef42a5ec5f03bbfa254cb01fad
					; v = 1 + 27
					0x1c
					; r
					0xdebaaa0cddb321b2dcaaf846d39605de7b97e77ba6106587855b9106cb104215
					; s
					0x61a22d94fa8b8a687ff9c911c844d1c016d1a685a9166858f9c7c1bc85128aca)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(fromHex("0x8743523d96a1b2cbe0c6909653a56da18ed484af")));
}

BOOST_AUTO_TEST_CASE(sha256_two_args)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(lit 0x20 "abcdefghijklmnopqrstuvwxyzABCDEF")
				(lit 0x40 "GHIJKLMNOPQRSTUVWXYZ0123456789?!")
				(sha256 0x20 0x40)
				(return 0x00 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("0xcf25a9fe3d86ae228c226c81d2d8c64c687cd6dc4586d10d8e7e4e5b6706d429")));
}

BOOST_AUTO_TEST_CASE(ripemd160_two_args)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(lit 0x20 "abcdefghijklmnopqrstuvwxyzABCDEF")
				(lit 0x40 "GHIJKLMNOPQRSTUVWXYZ0123456789?!")
				(ripemd160 0x20 0x40)
				(return 0x00 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("0x36c6b90a49e17d4c1e1b0e634ec74124d9b207da")));
}

BOOST_AUTO_TEST_CASE(sha256_one_arg)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(sha256 0x6162636465666768696a6b6c6d6e6f707172737475767778797a414243444546)
				(return 0x00 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("0xcfd2f1fad75a1978da0a444883db7251414b139f31f5a04704c291fdb0e175e6")));
}

BOOST_AUTO_TEST_CASE(ripemd160_one_arg)
{
	char const* sourceCode = R"(
		(returnlll
			(seq
				(ripemd160 0x6162636465666768696a6b6c6d6e6f707172737475767778797a414243444546)
				(return 0x00 0x20)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(
		fromHex("0xac5ab22e07b0fb80c69b6207902f725e2507e546")));
}

BOOST_AUTO_TEST_CASE(wei_szabo_finney_sophy)
{
	char const* sourceCode = R"(
		(returnlll
			(return (+ wei (+ szabo (+ finney sophy)))))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(1001001000000000001)));
}

BOOST_AUTO_TEST_CASE(shift_left)
{
	char const* sourceCode = R"(
		(returnlll
			(return (shl 1 8)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(256)));
}

BOOST_AUTO_TEST_CASE(shift_right)
{
	char const* sourceCode = R"(
		(returnlll
			(return (shr 65536 8)))
	)";
	compileAndRun(sourceCode);
	BOOST_CHECK(callFallback() == encodeArgs(u256(256)));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
