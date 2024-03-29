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
 * @author Marek Kotewicz <marek@sofdev.com>
 * @date 2014
 * Unit tests for the polynomial compiler JSON Interface output.
 */

#include <test/Options.h>
#include <libpolynomial/interface/CompilerStack.h>

#include <libdevcore/Exceptions.h>
#include <libdevcore/SwarmHash.h>

#include <libdevcore/JSON.h>

namespace dev
{
namespace polynomial
{
namespace test
{

class JSONInterfaceChecker
{
public:
	void checkInterface(std::string const& _code, std::string const& _contractName, std::string const& _expectedInterfaceString)
	{
		m_compilerStack.reset();
		m_compilerStack.setSources({{"", "pragma polynomial >=0.0;\n" + _code}});
		m_compilerStack.setSVMVersion(dev::test::Options::get().svmVersion());
		m_compilerStack.setOptimiserSettings(dev::test::Options::get().optimize);
		BOOST_REQUIRE_MESSAGE(m_compilerStack.parseAndAnalyze(), "Parsing contract failed");

		Json::Value generatedInterface = m_compilerStack.contractABI(_contractName);
		Json::Value expectedInterface;
		BOOST_REQUIRE(jsonParseStrict(_expectedInterfaceString, expectedInterface));
		BOOST_CHECK_MESSAGE(
			expectedInterface == generatedInterface,
			"Expected:\n" << expectedInterface.toStyledString() <<
			"\n but got:\n" << generatedInterface.toStyledString()
		);
	}

protected:
	CompilerStack m_compilerStack;
};

BOOST_FIXTURE_TEST_SUITE(PolynomialABIJSON, JSONInterfaceChecker)

BOOST_AUTO_TEST_CASE(basic_test)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a) public returns (uint d) { return a * 7; }
		}
	)";

	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
		"payable" : false,
		"stateMutability": "nonpayable",
		"type": "function",
		"inputs": [
		{
			"name": "a",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "d",
			"type": "uint256"
		}
		]
	}
	])";

	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(empty_contract)
{
	char const* sourceCode = R"(
		contract test { }
	)";
	char const* interface = "[]";

	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(multiple_methods)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a) public returns (uint d) { return a * 7; }
			function g(uint b) public returns (uint e) { return b * 8; }
		}
	)";

	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
		"payable" : false,
		"stateMutability": "nonpayable",
		"type": "function",
		"inputs": [
		{
			"name": "a",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "d",
			"type": "uint256"
		}
		]
	},
	{
		"name": "g",
		"constant": false,
		"payable" : false,
		"stateMutability": "nonpayable",
		"type": "function",
		"inputs": [
		{
			"name": "b",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "e",
			"type": "uint256"
		}
		]
	}
	])";

	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(multiple_params)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a, uint b) public returns (uint d) { return a + b; }
		}
	)";

	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
		"payable" : false,
		"stateMutability": "nonpayable",
		"type": "function",
		"inputs": [
		{
			"name": "a",
			"type": "uint256"
		},
		{
			"name": "b",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "d",
			"type": "uint256"
		}
		]
	}
	])";

	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(multiple_methods_order)
{
	// methods are expected to be in alphabetical order
	char const* sourceCode = R"(
		contract test {
			function f(uint a) public returns (uint d) { return a * 7; }
			function c(uint b) public returns (uint e) { return b * 8; }
		}
	)";

	char const* interface = R"([
	{
		"name": "c",
		"constant": false,
		"payable" : false,
		"stateMutability": "nonpayable",
		"type": "function",
		"inputs": [
		{
			"name": "b",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "e",
			"type": "uint256"
		}
		]
	},
	{
		"name": "f",
		"constant": false,
		"payable" : false,
		"stateMutability": "nonpayable",
		"type": "function",
		"inputs": [
		{
			"name": "a",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "d",
			"type": "uint256"
		}
		]
	}
	])";

	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(view_function)
{
	char const* sourceCode = R"(
		contract test {
			function foo(uint a, uint b) public returns (uint d) { return a + b; }
			function boo(uint32 a) public view returns(uint b) { return a * 4; }
		}
	)";

	char const* interface = R"([
	{
		"name": "foo",
		"constant": false,
		"payable" : false,
		"stateMutability": "nonpayable",
		"type": "function",
		"inputs": [
		{
			"name": "a",
			"type": "uint256"
		},
		{
			"name": "b",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "d",
			"type": "uint256"
		}
		]
	},
	{
		"name": "boo",
		"constant": true,
		"payable" : false,
		"stateMutability": "view",
		"type": "function",
		"inputs": [{
			"name": "a",
			"type": "uint32"
		}],
		"outputs": [
		{
			"name": "b",
			"type": "uint256"
		}
		]
	}
	])";

	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(pure_function)
{
	char const* sourceCode = R"(
		contract test {
			function foo(uint a, uint b) public returns (uint d) { return a + b; }
			function boo(uint32 a) public pure returns (uint b) { return a * 4; }
		}
	)";

	char const* interface = R"([
	{
		"name": "foo",
		"constant": false,
		"payable" : false,
		"stateMutability": "nonpayable",
		"type": "function",
		"inputs": [
		{
			"name": "a",
			"type": "uint256"
		},
		{
			"name": "b",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "d",
			"type": "uint256"
		}
		]
	},
	{
		"name": "boo",
		"constant": true,
		"payable" : false,
		"stateMutability": "pure",
		"type": "function",
		"inputs": [{
			"name": "a",
			"type": "uint32"
		}],
		"outputs": [
		{
			"name": "b",
			"type": "uint256"
		}
		]
	}
	])";

	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(events)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint a) public returns (uint d) { return a * 7; }
			event e1(uint b, address indexed c);
			event e2();
			event e2(uint a);
			event e3() anonymous;
		}
	)";
	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
		"payable" : false,
		"stateMutability": "nonpayable",
		"type": "function",
		"inputs": [
		{
			"name": "a",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "d",
			"type": "uint256"
		}
		]
	},
	{
		"name": "e1",
		"type": "event",
		"anonymous": false,
		"inputs": [
		{
			"indexed": false,
			"name": "b",
			"type": "uint256"
		},
		{
			"indexed": true,
			"name": "c",
			"type": "address"
		}
		]
	},
	{
		"name": "e2",
		"type": "event",
		"anonymous": false,
		"inputs": []
	},
	{
		"name": "e2",
		"type": "event",
		"anonymous": false,
		"inputs": [
		{
			"indexed": false,
			"name": "a",
			"type": "uint256"
		}
		]
	},
	{
		"name": "e3",
		"type": "event",
		"anonymous": true,
		"inputs": []
	}

	])";

	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(events_anonymous)
{
	char const* sourceCode = R"(
		contract test {
			event e() anonymous;
		}
	)";
	char const* interface = R"([
	{
		"name": "e",
		"type": "event",
		"anonymous": true,
		"inputs": []
	}

	])";

	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(inherited)
{
	char const* sourceCode = R"(
		contract Base {
			function baseFunction(uint p) public returns (uint i) { return p; }
			event baseEvent(bytes32 indexed evtArgBase);
		}
		contract Derived is Base {
			function derivedFunction(bytes32 p) public returns (bytes32 i) { return p; }
			event derivedEvent(uint indexed evtArgDerived);
		}
	)";

	char const* interface = R"([
	{
		"name": "baseFunction",
		"constant": false,
		"payable" : false,
		"stateMutability": "nonpayable",
		"type": "function",
		"inputs":
		[{
			"name": "p",
			"type": "uint256"
		}],
		"outputs":
		[{
			"name": "i",
			"type": "uint256"
		}]
	},
	{
		"name": "derivedFunction",
		"constant": false,
		"payable" : false,
		"stateMutability": "nonpayable",
		"type": "function",
		"inputs":
		[{
			"name": "p",
			"type": "bytes32"
		}],
		"outputs":
		[{
			"name": "i",
			"type": "bytes32"
		}]
	},
	{
		"name": "derivedEvent",
		"type": "event",
		"anonymous": false,
		"inputs":
		[{
			"indexed": true,
			"name": "evtArgDerived",
			"type": "uint256"
		}]
	},
	{
		"name": "baseEvent",
		"type": "event",
		"anonymous": false,
		"inputs":
		[{
			"indexed": true,
			"name": "evtArgBase",
			"type": "bytes32"
		}]
	}])";


	checkInterface(sourceCode, "Derived", interface);
}
BOOST_AUTO_TEST_CASE(empty_name_input_parameter_with_named_one)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint, uint k) public returns (uint ret_k, uint ret_g) {
				uint g = 8;
				ret_k = k;
				ret_g = g;
			}
		}
	)";

	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
		"payable" : false,
		"stateMutability": "nonpayable",
		"type": "function",
		"inputs": [
		{
			"name": "",
			"type": "uint256"
		},
		{
			"name": "k",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "ret_k",
			"type": "uint256"
		},
		{
			"name": "ret_g",
			"type": "uint256"
		}
		]
	}
	])";

	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(empty_name_return_parameter)
{
	char const* sourceCode = R"(
		contract test {
			function f(uint k) public returns (uint) {
				return k;
			}
		}
	)";

	char const* interface = R"([
	{
		"name": "f",
		"constant": false,
		"payable" : false,
		"stateMutability": "nonpayable",
		"type": "function",
		"inputs": [
		{
			"name": "k",
			"type": "uint256"
		}
		],
		"outputs": [
		{
			"name": "",
			"type": "uint256"
		}
		]
	}
	])";
	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(constructor_abi)
{
	char const* sourceCode = R"(
		contract test {
			constructor(uint param1, test param2, bool param3) public {}
		}
	)";

	char const* interface = R"([
	{
		"inputs": [
			{
				"name": "param1",
				"type": "uint256"
			},
			{
				"name": "param2",
				"type": "address"
			},
			{
				"name": "param3",
				"type": "bool"
			}
		],
		"payable": false,
		"stateMutability": "nonpayable",
		"type": "constructor"
	}
	])";
	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(payable_constructor_abi)
{
	char const* sourceCode = R"(
		contract test {
			constructor(uint param1, test param2, bool param3) public payable {}
		}
	)";

	char const* interface = R"([
	{
		"inputs": [
			{
				"name": "param1",
				"type": "uint256"
			},
			{
				"name": "param2",
				"type": "address"
			},
			{
				"name": "param3",
				"type": "bool"
			}
		],
		"payable": true,
		"stateMutability": "payable",
		"type": "constructor"
	}
	])";
	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(return_param_in_abi)
{
	// bug #1801
	char const* sourceCode = R"(
		contract test {
			enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
			constructor(ActionChoices param) public {}
			function ret() public returns (ActionChoices) {
				ActionChoices action = ActionChoices.GoLeft;
				return action;
			}
		}
	)";

	char const* interface = R"(
	[
		{
			"constant" : false,
			"payable" : false,
			"stateMutability": "nonpayable",
			"inputs" : [],
			"name" : "ret",
			"outputs" : [
				{
					"name" : "",
					"type" : "uint8"
				}
			],
			"type" : "function"
		},
		{
			"inputs": [
				{
					"name": "param",
					"type": "uint8"
				}
			],
			"payable": false,
			"stateMutability": "nonpayable",
			"type": "constructor"
		}
	]
	)";
	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(strings_and_arrays)
{
	// bug #1801
	char const* sourceCode = R"(
		contract test {
			function f(string calldata a, bytes calldata b, uint[] calldata c) external {}
		}
	)";

	char const* interface = R"(
	[
		{
			"constant" : false,
			"payable" : false,
			"stateMutability": "nonpayable",
			"name": "f",
			"inputs": [
				{ "name": "a", "type": "string" },
				{ "name": "b", "type": "bytes" },
				{ "name": "c", "type": "uint256[]" }
			],
			"outputs": [],
			"type" : "function"
		}
	]
	)";
	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(library_function)
{
	char const* sourceCode = R"(
		library test {
			struct StructType { uint a; }
			function f(StructType storage b, uint[] storage c, test d) public returns (uint[] memory e, StructType storage f) { f = f; }
			function f1(uint[] memory c, test d) public pure returns (uint[] memory e) {  }
		}
	)";

	char const* interface = R"(
	[
		{
			"constant" : true,
			"payable" : false,
			"stateMutability": "pure",
			"name": "f1",
			"inputs": [
				{ "name": "c", "type": "uint256[]" },
				{ "name": "d", "type": "test" }
			],
			"outputs": [
				{ "name": "e", "type": "uint256[]" }
			],
			"type" : "function"
		}
	]
	)";
	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(include_fallback_function)
{
	char const* sourceCode = R"(
		contract test {
			function() external {}
		}
	)";

	char const* interface = R"(
	[
		{
			"payable": false,
			"stateMutability": "nonpayable",
			"type" : "fallback"
		}
	]
	)";
	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(payable_function)
{
	char const* sourceCode = R"(
		contract test {
			function f() public {}
			function g() public payable {}
		}
	)";

	char const* interface = R"(
	[
		{
			"constant" : false,
			"payable": false,
			"stateMutability": "nonpayable",
			"inputs": [],
			"name": "f",
			"outputs": [],
			"type" : "function"
		},
		{
			"constant" : false,
			"payable": true,
			"stateMutability": "payable",
			"inputs": [],
			"name": "g",
			"outputs": [],
			"type" : "function"
		}
	]
	)";
	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(payable_fallback_function)
{
	char const* sourceCode = R"(
		contract test {
			function () external payable {}
		}
	)";

	char const* interface = R"(
	[
		{
			"payable": true,
			"stateMutability": "payable",
			"type" : "fallback"
		}
	]
	)";
	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(function_type)
{
	char const* sourceCode = R"(
		contract test {
			function g(function(uint) external returns (uint) x) public {}
		}
	)";

	char const* interface = R"(
	[
	{
		"constant" : false,
		"payable": false,
		"stateMutability": "nonpayable",
		"inputs": [{
			"name": "x",
			"type": "function"
		}],
		"name": "g",
		"outputs": [],
		"type" : "function"
	}
	]
	)";
	checkInterface(sourceCode, "test", interface);
}

BOOST_AUTO_TEST_CASE(return_structs)
{
	char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		contract C {
			struct S { uint a; T[] sub; }
			struct T { uint[2] x; }
			function f() public returns (uint x, S memory s) {
			}
		}
	)";
	char const* interface = R"(
	[{
		"constant" : false,
		"inputs" : [],
		"name" : "f",
		"outputs" : [
			{
			"name" : "x",
			"type" : "uint256"
			},
			{
			"components" : [
				{
					"name" : "a",
					"type" : "uint256"
				},
				{
					"components" : [
						{
						"name" : "x",
						"type" : "uint256[2]"
						}
					],
					"name" : "sub",
					"type" : "tuple[]"
				}
			],
			"name" : "s",
			"type" : "tuple"
			}
		],
		"payable" : false,
		"stateMutability" : "nonpayable",
		"type" : "function"
	}]
	)";
	checkInterface(sourceCode, "C", interface);
}

BOOST_AUTO_TEST_CASE(return_structs_with_contracts)
{
	char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		contract C {
			struct S { C[] x; C y; }
			function f() public returns (S memory s, C c) {
			}
		}
	)";
	char const* interface = R"(
	[{
		"constant": false,
		"inputs": [],
		"name": "f",
		"outputs": [
			{
				"components": [
					{
						"name": "x",
						"type": "address[]"
					},
					{
						"name": "y",
						"type": "address"
					}
				],
				"name": "s",
				"type": "tuple"
			},
			{
				"name": "c",
				"type": "address"
			}
		],
		"payable": false,
		"stateMutability" : "nonpayable",
		"type": "function"
	}]
	)";
	checkInterface(sourceCode, "C", interface);
}

BOOST_AUTO_TEST_CASE(event_structs)
{
	char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		contract C {
			struct S { uint a; T[] sub; bytes b; }
			struct T { uint[2] x; }
			event E(T t, S s);
		}
	)";
	char const *interface = R"(
		[{
		"anonymous": false,
		"inputs": [
			{
				"components": [
					{
						"name": "x",
						"type": "uint256[2]"
					}
				],
				"indexed": false,
				"name": "t",
				"type": "tuple"
			},
			{
				"components": [
					{
						"name": "a",
						"type": "uint256"
					},
					{
						"components": [
							{
								"name": "x",
								"type": "uint256[2]"
							}
						],
						"name": "sub",
						"type": "tuple[]"
					},
					{
						"name": "b",
						"type": "bytes"
					}
				],
				"indexed": false,
				"name": "s",
				"type": "tuple"
			}
		],
		"name": "E",
		"type": "event"
	}]
	)";
	checkInterface(sourceCode, "C", interface);
}

BOOST_AUTO_TEST_CASE(structs_in_libraries)
{
	char const* sourceCode = R"(
		pragma experimental ABIEncoderV2;
		library L {
			struct S { uint a; T[] sub; bytes b; }
			struct T { uint[2] x; }
			function f(L.S storage s) public view {}
			function g(L.S memory s) public view {}
		}
	)";
	char const* interface = R"(
	[{
		"constant": true,
		"inputs": [
			{
				"components": [
					{
						"name": "a",
						"type": "uint256"
					},
					{
						"components": [
							{
								"name": "x",
								"type": "uint256[2]"
							}
						],
						"name": "sub",
						"type": "tuple[]"
					},
					{
						"name": "b",
						"type": "bytes"
					}
				],
				"name": "s",
				"type": "tuple"
			}
		],
		"name": "g",
		"outputs": [],
		"payable": false,
		"stateMutability": "view",
		"type": "function"
	}])";
	checkInterface(sourceCode, "L", interface);
}

BOOST_AUTO_TEST_SUITE_END()

}
}
}
