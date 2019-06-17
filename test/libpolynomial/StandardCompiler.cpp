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
 * Unit tests for interface/StandardCompiler.h.
 */

#include <string>
#include <boost/test/unit_test.hpp>
#include <libpolynomial/interface/StandardCompiler.h>
#include <libdevcore/JSON.h>

#include "../Metadata.h"

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

bool containsAtMostWarnings(Json::Value const& _compilerResult)
{
	if (!_compilerResult.isMember("errors"))
		return true;

	for (auto const& error: _compilerResult["errors"])
	{
		BOOST_REQUIRE(error.isObject());
		BOOST_REQUIRE(error["severity"].isString());
		if (error["severity"].asString() != "warning")
			return false;
	}

	return true;
}

Json::Value getContractResult(Json::Value const& _compilerResult, string const& _file, string const& _name)
{
	if (
		!_compilerResult["contracts"].isObject() ||
		!_compilerResult["contracts"][_file].isObject() ||
		!_compilerResult["contracts"][_file][_name].isObject()
	)
		return Json::Value();
	return _compilerResult["contracts"][_file][_name];
}

Json::Value compile(string const& _input)
{
	StandardCompiler compiler;
	string output = compiler.compile(_input);
	Json::Value ret;
	BOOST_REQUIRE(jsonParseStrict(output, ret));
	return ret;
}

} // end anonymous namespace

BOOST_AUTO_TEST_SUITE(StandardCompiler)

BOOST_AUTO_TEST_CASE(assume_object_input)
{
	Json::Value result;

	/// Use the native JSON interface of StandardCompiler to trigger these
	polynomial::StandardCompiler compiler;
	result = compiler.compile(Json::Value());
	BOOST_CHECK(containsError(result, "JSONError", "Input is not a JSON object."));
	result = compiler.compile(Json::Value("INVALID"));
	BOOST_CHECK(containsError(result, "JSONError", "Input is not a JSON object."));

	/// Use the string interface of StandardCompiler to trigger these
	result = compile("");
	BOOST_CHECK(containsError(result, "JSONError", "* Line 1, Column 1\n  Syntax error: value, object or array expected.\n* Line 1, Column 1\n  A valid JSON document must be either an array or an object value.\n"));
	result = compile("invalid");
	BOOST_CHECK(containsError(result, "JSONError", "* Line 1, Column 1\n  Syntax error: value, object or array expected.\n* Line 1, Column 2\n  Extra non-whitespace after JSON value.\n"));
	result = compile("\"invalid\"");
	BOOST_CHECK(containsError(result, "JSONError", "* Line 1, Column 1\n  A valid JSON document must be either an array or an object value.\n"));
	BOOST_CHECK(!containsError(result, "JSONError", "* Line 1, Column 1\n  Syntax error: value, object or array expected.\n"));
	result = compile("{}");
	BOOST_CHECK(!containsError(result, "JSONError", "* Line 1, Column 1\n  Syntax error: value, object or array expected.\n"));
	BOOST_CHECK(!containsAtMostWarnings(result));
}

BOOST_AUTO_TEST_CASE(invalid_language)
{
	char const* input = R"(
	{
		"language": "INVALID"
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "Only \"Polynomial\" is supported as a language."));
}

BOOST_AUTO_TEST_CASE(valid_language)
{
	char const* input = R"(
	{
		"language": "Polynomial"
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(!containsError(result, "JSONError", "Only \"Polynomial\" is supported as a language."));
}

BOOST_AUTO_TEST_CASE(no_sources)
{
	char const* input = R"(
	{
		"language": "Polynomial"
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "No input sources specified."));
}

BOOST_AUTO_TEST_CASE(no_sources_empty_object)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"sources": {}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "No input sources specified."));
}

BOOST_AUTO_TEST_CASE(no_sources_empty_array)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"sources": []
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "\"sources\" is not a JSON object."));
}

BOOST_AUTO_TEST_CASE(sources_is_array)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"sources": ["aa", "bb"]
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "\"sources\" is not a JSON object."));
}

BOOST_AUTO_TEST_CASE(unexpected_trailing_test)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"sources": {
			"A": {
				"content": "contract A { function f() {} }"
			}
		}
	}
	}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "* Line 10, Column 2\n  Extra non-whitespace after JSON value.\n"));
}


BOOST_AUTO_TEST_CASE(smoke_test)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"sources": {
			"empty": {
				"content": ""
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
}

BOOST_AUTO_TEST_CASE(basic_compilation)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"sources": {
			"fileA": {
				"content": "contract A { }"
			}
		},
		"settings": {
			"outputSelection": {
				"fileA": {
					"A": [ "abi", "devdoc", "userdoc", "svm.bytecode", "svm.assembly", "svm.gasEstimates", "metadata" ],
					"": [ "legacyAST" ]
				}
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json::Value contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["abi"].isArray());
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["abi"]), "[]");
	BOOST_CHECK(contract["devdoc"].isObject());
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["devdoc"]), "{\"methods\":{}}");
	BOOST_CHECK(contract["userdoc"].isObject());
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["userdoc"]), "{\"methods\":{}}");
	BOOST_CHECK(contract["svm"].isObject());
	/// @TODO check svm.methodIdentifiers, legacyAssembly, bytecode, deployedBytecode
	BOOST_CHECK(contract["svm"]["bytecode"].isObject());
	BOOST_CHECK(contract["svm"]["bytecode"]["object"].isString());
	BOOST_CHECK_EQUAL(
		dev::test::bytecodeSansMetadata(contract["svm"]["bytecode"]["object"].asString()),
		"6080604052348015600f57600080fd5b50603580601d6000396000f3006080604052600080fd00"
	);
	BOOST_CHECK(contract["svm"]["assembly"].isString());
	BOOST_CHECK(contract["svm"]["assembly"].asString().find(
		"    /* \"fileA\":0:14  contract A { } */\n  mstore(0x40, 0x80)\n  "
		"callvalue\n    /* \"--CODEGEN--\":8:17   */\n  dup1\n    "
		"/* \"--CODEGEN--\":5:7   */\n  iszero\n  tag_1\n  jumpi\n    "
		"/* \"--CODEGEN--\":30:31   */\n  0x0\n    /* \"--CODEGEN--\":27:28   */\n  "
		"dup1\n    /* \"--CODEGEN--\":20:32   */\n  revert\n    /* \"--CODEGEN--\":5:7   */\n"
		"tag_1:\n    /* \"fileA\":0:14  contract A { } */\n  pop\n  dataSize(sub_0)\n  dup1\n  "
		"dataOffset(sub_0)\n  0x0\n  codecopy\n  0x0\n  return\nstop\n\nsub_0: assembly {\n        "
		"/* \"fileA\":0:14  contract A { } */\n      mstore(0x40, 0x80)\n      0x0\n      "
		"dup1\n      revert\n\n    auxdata: 0xa165627a7a72305820"
	) == 0);
	BOOST_CHECK(contract["svm"]["gasEstimates"].isObject());
	BOOST_CHECK_EQUAL(
		dev::jsonCompactPrint(contract["svm"]["gasEstimates"]),
		"{\"creation\":{\"codeDepositCost\":\"10600\",\"executionCost\":\"66\",\"totalCost\":\"10666\"}}"
	);
	BOOST_CHECK(contract["metadata"].isString());
	BOOST_CHECK(dev::test::isValidMetadata(contract["metadata"].asString()));
	BOOST_CHECK(result["sources"].isObject());
	BOOST_CHECK(result["sources"]["fileA"].isObject());
	BOOST_CHECK(result["sources"]["fileA"]["legacyAST"].isObject());
	BOOST_CHECK_EQUAL(
		dev::jsonCompactPrint(result["sources"]["fileA"]["legacyAST"]),
		"{\"attributes\":{\"absolutePath\":\"fileA\",\"exportedSymbols\":{\"A\":[1]}},\"children\":"
		"[{\"attributes\":{\"baseContracts\":[null],\"contractDependencies\":[null],\"contractKind\":\"contract\","
		"\"documentation\":null,\"fullyImplemented\":true,\"linearizedBaseContracts\":[1],\"name\":\"A\",\"nodes\":[null],\"scope\":2},"
		"\"id\":1,\"name\":\"ContractDefinition\",\"src\":\"0:14:0\"}],\"id\":2,\"name\":\"SourceUnit\",\"src\":\"0:14:0\"}"
	);
}

BOOST_AUTO_TEST_CASE(compilation_error)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"settings": {
			"outputSelection": {
				"fileA": {
					"A": [
						"abi"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "contract A { function }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(result.isMember("errors"));
	BOOST_CHECK(result["errors"].size() >= 1);
	for (auto const& error: result["errors"])
	{
		BOOST_REQUIRE(error.isObject());
		BOOST_REQUIRE(error["message"].isString());
		if (error["message"].asString().find("pre-release compiler") == string::npos)
		{
			BOOST_CHECK_EQUAL(
				dev::jsonCompactPrint(error),
				"{\"component\":\"general\",\"formattedMessage\":\"fileA:1:23: ParserError: Expected identifier, got 'RBrace'\\n"
				"contract A { function }\\n                      ^\\n\",\"message\":\"Expected identifier, got 'RBrace'\","
				"\"severity\":\"error\",\"sourceLocation\":{\"end\":22,\"file\":\"fileA\",\"start\":22},\"type\":\"ParserError\"}"
			);
		}
	}
}

BOOST_AUTO_TEST_CASE(output_selection_explicit)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"settings": {
			"outputSelection": {
				"fileA": {
					"A": [
						"abi"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "contract A { }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json::Value contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["abi"].isArray());
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["abi"]), "[]");
}

BOOST_AUTO_TEST_CASE(output_selection_all_contracts)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"settings": {
			"outputSelection": {
				"fileA": {
					"*": [
						"abi"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "contract A { }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json::Value contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["abi"].isArray());
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["abi"]), "[]");
}

BOOST_AUTO_TEST_CASE(output_selection_all_files_single_contract)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"settings": {
			"outputSelection": {
				"*": {
					"A": [
						"abi"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "contract A { }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json::Value contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["abi"].isArray());
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["abi"]), "[]");
}

BOOST_AUTO_TEST_CASE(output_selection_all_files_all_contracts)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"settings": {
			"outputSelection": {
				"*": {
					"*": [
						"abi"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "contract A { }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json::Value contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["abi"].isArray());
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["abi"]), "[]");
}

BOOST_AUTO_TEST_CASE(output_selection_dependent_contract)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"settings": {
			"outputSelection": {
				"*": {
					"A": [
						"abi"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "contract B { } contract A { function f() { new B(); } }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json::Value contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["abi"].isArray());
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["abi"]), "[{\"constant\":false,\"inputs\":[],\"name\":\"f\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]");
}

BOOST_AUTO_TEST_CASE(output_selection_dependent_contract_with_import)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"settings": {
			"outputSelection": {
				"*": {
					"A": [
						"abi"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "import \"fileB\"; contract A { function f() { new B(); } }"
			},
			"fileB": {
				"content": "contract B { }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json::Value contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["abi"].isArray());
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["abi"]), "[{\"constant\":false,\"inputs\":[],\"name\":\"f\",\"outputs\":[],\"payable\":false,\"stateMutability\":\"nonpayable\",\"type\":\"function\"}]");
}

BOOST_AUTO_TEST_CASE(filename_with_colon)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"settings": {
			"outputSelection": {
				"http://octonion.institute/susy-lang/polynomial/std/StandardToken.pol": {
					"A": [
						"abi"
					]
				}
			}
		},
		"sources": {
			"http://octonion.institute/susy-lang/polynomial/std/StandardToken.pol": {
				"content": "contract A { }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json::Value contract = getContractResult(result, "http://octonion.institute/susy-lang/polynomial/std/StandardToken.pol", "A");
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["abi"].isArray());
	BOOST_CHECK_EQUAL(dev::jsonCompactPrint(contract["abi"]), "[]");
}

BOOST_AUTO_TEST_CASE(library_filename_with_colon)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"settings": {
			"outputSelection": {
				"fileA": {
					"A": [
						"svm.bytecode"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "import \"git:library.pol\"; contract A { function f() returns (uint) { return L.g(); } }"
			},
			"git:library.pol": {
				"content": "library L { function g() returns (uint) { return 1; } }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json::Value contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["svm"]["bytecode"].isObject());
	BOOST_CHECK(contract["svm"]["bytecode"]["linkReferences"].isObject());
	BOOST_CHECK(contract["svm"]["bytecode"]["linkReferences"]["git:library.pol"].isObject());
	BOOST_CHECK(contract["svm"]["bytecode"]["linkReferences"]["git:library.pol"]["L"].isArray());
	BOOST_CHECK(contract["svm"]["bytecode"]["linkReferences"]["git:library.pol"]["L"][0].isObject());
}

BOOST_AUTO_TEST_CASE(libraries_invalid_top_level)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"settings": {
			"libraries": "42"
		},
		"sources": {
			"empty": {
				"content": ""
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "\"libraries\" is not a JSON object."));
}

BOOST_AUTO_TEST_CASE(libraries_invalid_entry)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"settings": {
			"libraries": {
				"L": "42"
			}
		},
		"sources": {
			"empty": {
				"content": ""
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "library entry is not a JSON object."));
}

BOOST_AUTO_TEST_CASE(libraries_invalid_hex)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"settings": {
			"libraries": {
				"library.pol": {
					"L": "0x4200000000000000000000000000000000000xx1"
				}
			}
		},
		"sources": {
			"empty": {
				"content": ""
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "Invalid library address (\"0x4200000000000000000000000000000000000xx1\") supplied."));
}

BOOST_AUTO_TEST_CASE(libraries_invalid_length)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"settings": {
			"libraries": {
				"library.pol": {
					"L1": "0x42",
					"L2": "0x4200000000000000000000000000000000000001ff"
				}
			}
		},
		"sources": {
			"empty": {
				"content": ""
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "Library address is of invalid length."));
}

BOOST_AUTO_TEST_CASE(libraries_missing_hex_prefix)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"settings": {
			"libraries": {
				"library.pol": {
					"L": "4200000000000000000000000000000000000001"
				}
			}
		},
		"sources": {
			"empty": {
				"content": ""
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsError(result, "JSONError", "Library address is not prefixed with \"0x\"."));
}

BOOST_AUTO_TEST_CASE(library_linking)
{
	char const* input = R"(
	{
		"language": "Polynomial",
		"settings": {
			"libraries": {
				"library.pol": {
					"L": "0x4200000000000000000000000000000000000001"
				}
			},
			"outputSelection": {
				"fileA": {
					"A": [
						"svm.bytecode"
					]
				}
			}
		},
		"sources": {
			"fileA": {
				"content": "import \"library.pol\"; import \"library2.pol\"; contract A { function f() returns (uint) { L2.g(); return L.g(); } }"
			},
			"library.pol": {
				"content": "library L { function g() returns (uint) { return 1; } }"
			},
			"library2.pol": {
				"content": "library L2 { function g() { } }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(containsAtMostWarnings(result));
	Json::Value contract = getContractResult(result, "fileA", "A");
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["svm"]["bytecode"].isObject());
	BOOST_CHECK(contract["svm"]["bytecode"]["linkReferences"].isObject());
	BOOST_CHECK(!contract["svm"]["bytecode"]["linkReferences"]["library.pol"].isObject());
	BOOST_CHECK(contract["svm"]["bytecode"]["linkReferences"]["library2.pol"].isObject());
	BOOST_CHECK(contract["svm"]["bytecode"]["linkReferences"]["library2.pol"]["L2"].isArray());
	BOOST_CHECK(contract["svm"]["bytecode"]["linkReferences"]["library2.pol"]["L2"][0].isObject());
}

BOOST_AUTO_TEST_CASE(svm_version)
{
	auto inputForVersion = [](string const& _version)
	{
		return R"(
			{
				"language": "Polynomial",
				"sources": { "fileA": { "content": "contract A { }" } },
				"settings": {
					)" + _version + R"(
					"outputSelection": {
						"fileA": {
							"A": [ "metadata" ]
						}
					}
				}
			}
		)";
	};
	Json::Value result;
	result = compile(inputForVersion("\"svmVersion\": \"homestead\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].asString().find("\"svmVersion\":\"homestead\"") != string::npos);
	result = compile(inputForVersion("\"svmVersion\": \"tangerineWhistle\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].asString().find("\"svmVersion\":\"tangerineWhistle\"") != string::npos);
	result = compile(inputForVersion("\"svmVersion\": \"spuriousDragon\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].asString().find("\"svmVersion\":\"spuriousDragon\"") != string::npos);
	result = compile(inputForVersion("\"svmVersion\": \"byzantium\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].asString().find("\"svmVersion\":\"byzantium\"") != string::npos);
	result = compile(inputForVersion("\"svmVersion\": \"constantinople\","));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].asString().find("\"svmVersion\":\"constantinople\"") != string::npos);
	// test default
	result = compile(inputForVersion(""));
	BOOST_CHECK(result["contracts"]["fileA"]["A"]["metadata"].asString().find("\"svmVersion\":\"byzantium\"") != string::npos);
	// test invalid
	result = compile(inputForVersion("\"svmVersion\": \"invalid\","));
	BOOST_CHECK(result["errors"][0]["message"].asString() == "Invalid SVM version requested.");
}


BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
