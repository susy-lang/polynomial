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
 * @date 2014
 * Unit tests for the polynomial expression compiler.
 */

#include <string>

#include <libdevcore/Log.h>
#include <libpolynomial/parsing/Scanner.h>
#include <libpolynomial/parsing/Parser.h>
#include <libpolynomial/analysis/NameAndTypeResolver.h>
#include <libpolynomial/codegen/CompilerContext.h>
#include <libpolynomial/codegen/ExpressionCompiler.h>
#include <libpolynomial/ast/AST.h>
#include <libpolynomial/analysis/TypeChecker.h>
#include "../TestHelper.h"

using namespace std;

namespace dev
{
namespace polynomial
{
namespace test
{

namespace
{

/// Helper class that extracts the first expression in an AST.
class FirstExpressionExtractor: private ASTVisitor
{
public:
	FirstExpressionExtractor(ASTNode& _node): m_expression(nullptr) { _node.accept(*this); }
	Expression* expression() const { return m_expression; }
private:
	virtual bool visit(Assignment& _expression) override { return checkExpression(_expression); }
	virtual bool visit(UnaryOperation& _expression) override { return checkExpression(_expression); }
	virtual bool visit(BinaryOperation& _expression) override { return checkExpression(_expression); }
	virtual bool visit(FunctionCall& _expression) override { return checkExpression(_expression); }
	virtual bool visit(MemberAccess& _expression) override { return checkExpression(_expression); }
	virtual bool visit(IndexAccess& _expression) override { return checkExpression(_expression); }
	virtual bool visit(Identifier& _expression) override { return checkExpression(_expression); }
	virtual bool visit(ElementaryTypeNameExpression& _expression) override { return checkExpression(_expression); }
	virtual bool visit(Literal& _expression) override { return checkExpression(_expression); }
	bool checkExpression(Expression& _expression)
	{
		if (m_expression == nullptr)
			m_expression = &_expression;
		return false;
	}
private:
	Expression* m_expression;
};

Declaration const& resolveDeclaration(
	SourceUnit const& _sourceUnit,
	vector<string> const& _namespacedName,
	NameAndTypeResolver const& _resolver
)
{
	ASTNode const* scope = &_sourceUnit;
	// bracers are required, cause msvc couldnt handle this macro in for statement
	for (string const& namePart: _namespacedName)
	{
		auto declarations = _resolver.resolveName(namePart, scope);
		BOOST_REQUIRE(!declarations.empty());
		BOOST_REQUIRE(scope = *declarations.begin());
	}
	BOOST_REQUIRE(scope);
	return dynamic_cast<Declaration const&>(*scope);
}

bytes compileFirstExpression(
	const string& _sourceCode,
	vector<vector<string>> _functions = {},
	vector<vector<string>> _localVariables = {},
	vector<shared_ptr<MagicVariableDeclaration const>> _globalDeclarations = {}
)
{
	ASTPointer<SourceUnit> sourceUnit;
	try
	{
		ErrorList errors;
		sourceUnit = Parser(errors).parse(make_shared<Scanner>(CharStream(_sourceCode)));
		if (!sourceUnit)
			return bytes();
	}
	catch(boost::exception const& _e)
	{
		auto msg = std::string("Parsing source code failed with: \n") + boost::diagnostic_information(_e);
		BOOST_FAIL(msg);
	}

	vector<Declaration const*> declarations;
	declarations.reserve(_globalDeclarations.size() + 1);
	for (ASTPointer<Declaration const> const& variable: _globalDeclarations)
		declarations.push_back(variable.get());

	ErrorList errors;
	NameAndTypeResolver resolver(declarations, errors);
	resolver.registerDeclarations(*sourceUnit);

	vector<ContractDefinition const*> inheritanceHierarchy;
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			SOF_TEST_REQUIRE_NO_THROW(resolver.resolveNamesAndTypes(*contract), "Repolving names failed");
			inheritanceHierarchy = vector<ContractDefinition const*>(1, contract);
		}
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			TypeChecker typeChecker(errors);
			BOOST_REQUIRE(typeChecker.checkTypeRequirements(*contract));
		}
	for (ASTPointer<ASTNode> const& node: sourceUnit->nodes())
		if (ContractDefinition* contract = dynamic_cast<ContractDefinition*>(node.get()))
		{
			FirstExpressionExtractor extractor(*contract);
			BOOST_REQUIRE(extractor.expression() != nullptr);

			CompilerContext context;
			context.resetVisitedNodes(contract);
			context.setInheritanceHierarchy(inheritanceHierarchy);
			unsigned parametersSize = _localVariables.size(); // assume they are all one slot on the stack
			context.adjustStackOffset(parametersSize);
			for (vector<string> const& variable: _localVariables)
				context.addVariable(
					dynamic_cast<VariableDeclaration const&>(resolveDeclaration(*sourceUnit, variable, resolver)),
					parametersSize--
				);

			ExpressionCompiler(context).compile(*extractor.expression());

			for (vector<string> const& function: _functions)
				context << context.functionEntryLabel(dynamic_cast<FunctionDefinition const&>(
					resolveDeclaration(*sourceUnit, function, resolver)
				));
			bytes instructions = context.assembledObject().bytecode;
			// debug
			// cout << sof::disassemble(instructions) << endl;
			return instructions;
		}
	BOOST_FAIL("No contract found in source.");
	return bytes();
}

} // end anonymous namespace

BOOST_AUTO_TEST_SUITE(PolynomialExpressionCompiler)

BOOST_AUTO_TEST_CASE(literal_true)
{
	char const* sourceCode = "contract test {\n"
							 "  function f() { var x = true; }"
							 "}\n";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({byte(sof::Instruction::PUSH1), 0x1});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(literal_false)
{
	char const* sourceCode = "contract test {\n"
							 "  function f() { var x = false; }"
							 "}\n";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({byte(sof::Instruction::PUSH1), 0x0});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(int_literal)
{
	char const* sourceCode = "contract test {\n"
							 "  function f() { var x = 0x12345678901234567890; }"
							 "}\n";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({byte(sof::Instruction::PUSH10), 0x12, 0x34, 0x56, 0x78, 0x90,
													   0x12, 0x34, 0x56, 0x78, 0x90});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(int_with_wei_sophy_subdenomination)
{
	char const* sourceCode = R"(
		contract test {
			function test ()
			{
				 var x = 1 wei;
			}
		})";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({byte(sof::Instruction::PUSH1), 0x1});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(int_with_szabo_sophy_subdenomination)
{
	char const* sourceCode = R"(
		contract test {
			function test ()
			{
				 var x = 1 szabo;
			}
		})";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({byte(sof::Instruction::PUSH5), 0xe8, 0xd4, 0xa5, 0x10, 0x00});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(int_with_finney_sophy_subdenomination)
{
	char const* sourceCode = R"(
		contract test {
			function test ()
			{
				 var x = 1 finney;
			}
		})";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({byte(sof::Instruction::PUSH7), 0x3, 0x8d, 0x7e, 0xa4, 0xc6, 0x80, 0x00});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(int_with_sophy_sophy_subdenomination)
{
	char const* sourceCode = R"(
		contract test {
			function test ()
			{
				 var x = 1 sophy;
			}
		})";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({byte(sof::Instruction::PUSH8), 0xd, 0xe0, 0xb6, 0xb3, 0xa7, 0x64, 0x00, 0x00});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(comparison)
{
	char const* sourceCode = "contract test {\n"
							 "  function f() { var x = (0x10aa < 0x11aa) != true; }"
							 "}\n";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({byte(sof::Instruction::PUSH1), 0x1,
					   byte(sof::Instruction::PUSH2), 0x11, 0xaa,
					   byte(sof::Instruction::PUSH2), 0x10, 0xaa,
					   byte(sof::Instruction::LT),
					   byte(sof::Instruction::EQ),
					   byte(sof::Instruction::ISZERO)});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(short_circuiting)
{
	char const* sourceCode = "contract test {\n"
							 "  function f() { var x = true != (4 <= 8 + 10 || 9 != 2); }"
							 "}\n";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation({byte(sof::Instruction::PUSH1), 0x12, // 8 + 10
					   byte(sof::Instruction::PUSH1), 0x4,
					   byte(sof::Instruction::GT),
					   byte(sof::Instruction::ISZERO), // after this we have 4 <= 8 + 10
					   byte(sof::Instruction::DUP1),
					   byte(sof::Instruction::PUSH1), 0x11,
					   byte(sof::Instruction::JUMPI), // short-circuit if it is true
					   byte(sof::Instruction::POP),
					   byte(sof::Instruction::PUSH1), 0x2,
					   byte(sof::Instruction::PUSH1), 0x9,
					   byte(sof::Instruction::EQ),
					   byte(sof::Instruction::ISZERO), // after this we have 9 != 2
					   byte(sof::Instruction::JUMPDEST),
					   byte(sof::Instruction::PUSH1), 0x1,
					   byte(sof::Instruction::EQ),
					   byte(sof::Instruction::ISZERO)});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(arithmetics)
{
	char const* sourceCode = "contract test {\n"
							 "  function f(uint y) { var x = ((((((((y ^ 8) & 7) | 6) - 5) + 4) % 3) / 2) * 1); }"
							 "}\n";
	bytes code = compileFirstExpression(sourceCode, {}, {{"test", "f", "y"}, {"test", "f", "x"}});
	bytes expectation({byte(sof::Instruction::PUSH1), 0x1,
					   byte(sof::Instruction::PUSH1), 0x2,
					   byte(sof::Instruction::PUSH1), 0x3,
					   byte(sof::Instruction::PUSH1), 0x4,
					   byte(sof::Instruction::PUSH1), 0x5,
					   byte(sof::Instruction::PUSH1), 0x6,
					   byte(sof::Instruction::PUSH1), 0x7,
					   byte(sof::Instruction::PUSH1), 0x8,
					   byte(sof::Instruction::DUP10),
					   byte(sof::Instruction::XOR),
					   byte(sof::Instruction::AND),
					   byte(sof::Instruction::OR),
					   byte(sof::Instruction::SUB),
					   byte(sof::Instruction::ADD),
					   byte(sof::Instruction::MOD),
					   byte(sof::Instruction::DIV),
					   byte(sof::Instruction::MUL)});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(unary_operators)
{
	char const* sourceCode = "contract test {\n"
							 "  function f(int y) { var x = !(~+- y == 2); }"
							 "}\n";
	bytes code = compileFirstExpression(sourceCode, {}, {{"test", "f", "y"}, {"test", "f", "x"}});

	bytes expectation({byte(sof::Instruction::PUSH1), 0x2,
					   byte(sof::Instruction::DUP3),
					   byte(sof::Instruction::PUSH1), 0x0,
					   byte(sof::Instruction::SUB),
					   byte(sof::Instruction::NOT),
					   byte(sof::Instruction::EQ),
					   byte(sof::Instruction::ISZERO)});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(unary_inc_dec)
{
	char const* sourceCode = "contract test {\n"
							 "  function f(uint a) { var x = --a ^ (a-- ^ (++a ^ a++)); }"
							 "}\n";
	bytes code = compileFirstExpression(sourceCode, {}, {{"test", "f", "a"}, {"test", "f", "x"}});

	// Stack: a, x
	bytes expectation({byte(sof::Instruction::DUP2),
					   byte(sof::Instruction::DUP1),
					   byte(sof::Instruction::PUSH1), 0x1,
					   byte(sof::Instruction::ADD),
					   // Stack here: a x a (a+1)
					   byte(sof::Instruction::SWAP3),
					   byte(sof::Instruction::POP), // first ++
					   // Stack here: (a+1) x a
					   byte(sof::Instruction::DUP3),
					   byte(sof::Instruction::PUSH1), 0x1,
					   byte(sof::Instruction::ADD),
					   // Stack here: (a+1) x a (a+2)
					   byte(sof::Instruction::SWAP3),
					   byte(sof::Instruction::POP),
					   // Stack here: (a+2) x a
					   byte(sof::Instruction::DUP3), // second ++
					   byte(sof::Instruction::XOR),
					   // Stack here: (a+2) x a^(a+2)
					   byte(sof::Instruction::DUP3),
					   byte(sof::Instruction::DUP1),
					   byte(sof::Instruction::PUSH1), 0x1,
					   byte(sof::Instruction::SWAP1),
					   byte(sof::Instruction::SUB),
					   // Stack here: (a+2) x a^(a+2) (a+2) (a+1)
					   byte(sof::Instruction::SWAP4),
					   byte(sof::Instruction::POP), // first --
					   byte(sof::Instruction::XOR),
					   // Stack here: (a+1) x a^(a+2)^(a+2)
					   byte(sof::Instruction::DUP3),
					   byte(sof::Instruction::PUSH1), 0x1,
					   byte(sof::Instruction::SWAP1),
					   byte(sof::Instruction::SUB),
					   // Stack here: (a+1) x a^(a+2)^(a+2) a
					   byte(sof::Instruction::SWAP3),
					   byte(sof::Instruction::POP), // second ++
					   // Stack here: a x a^(a+2)^(a+2)
					   byte(sof::Instruction::DUP3), // will change
					   byte(sof::Instruction::XOR)});
					   // Stack here: a x a^(a+2)^(a+2)^a
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(assignment)
{
	char const* sourceCode = "contract test {\n"
							 "  function f(uint a, uint b) { (a += b) * 2; }"
							 "}\n";
	bytes code = compileFirstExpression(sourceCode, {}, {{"test", "f", "a"}, {"test", "f", "b"}});

	// Stack: a, b
	bytes expectation({byte(sof::Instruction::PUSH1), 0x2,
					   byte(sof::Instruction::DUP2),
					   byte(sof::Instruction::DUP4),
					   byte(sof::Instruction::ADD),
					   // Stack here: a b 2 a+b
					   byte(sof::Instruction::SWAP3),
					   byte(sof::Instruction::POP),
					   byte(sof::Instruction::DUP3),
					   // Stack here: a+b b 2 a+b
					   byte(sof::Instruction::MUL)});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(function_call)
{
	char const* sourceCode = "contract test {\n"
							 "  function f(uint a, uint b) { a += g(a + 1, b) * 2; }\n"
							 "  function g(uint a, uint b) returns (uint c) {}\n"
							 "}\n";
	bytes code = compileFirstExpression(sourceCode, {{"test", "g"}},
										{{"test", "f", "a"}, {"test", "f", "b"}});

	// Stack: a, b
	bytes expectation({byte(sof::Instruction::PUSH1), 0x02,
					   byte(sof::Instruction::PUSH1), 0x0c,
					   byte(sof::Instruction::PUSH1), 0x01,
					   byte(sof::Instruction::DUP5),
					   byte(sof::Instruction::ADD),
					   // Stack here: a b 2 <ret label> (a+1)
					   byte(sof::Instruction::DUP4),
					   byte(sof::Instruction::PUSH1), 0x13,
					   byte(sof::Instruction::JUMP),
					   byte(sof::Instruction::JUMPDEST),
					   // Stack here: a b 2 g(a+1, b)
					   byte(sof::Instruction::MUL),
					   // Stack here: a b g(a+1, b)*2
					   byte(sof::Instruction::DUP3),
					   byte(sof::Instruction::ADD),
					   // Stack here: a b a+g(a+1, b)*2
					   byte(sof::Instruction::SWAP2),
					   byte(sof::Instruction::POP),
					   byte(sof::Instruction::DUP2),
					   byte(sof::Instruction::JUMPDEST)});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(negative_literals_8bits)
{
	char const* sourceCode = "contract test {\n"
							 "  function f() { int8 x = -0x80; }\n"
							 "}\n";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation(bytes({byte(sof::Instruction::PUSH32)}) + bytes(31, 0xff) + bytes(1, 0x80));
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(negative_literals_16bits)
{
	char const* sourceCode = "contract test {\n"
							 "  function f() { int64 x = ~0xabc; }\n"
							 "}\n";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation(bytes({byte(sof::Instruction::PUSH32)}) + bytes(30, 0xff) + bytes{0xf5, 0x43});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(intermediately_overflowing_literals)
{
	// first literal itself is too large for 256 bits but it fits after all constant operations
	// have been applied
	char const* sourceCode = "contract test {\n"
							 "  function f() { var x = (0xffffffffffffffffffffffffffffffffffffffff * 0xffffffffffffffffffffffffff01) & 0xbf; }\n"
							 "}\n";
	bytes code = compileFirstExpression(sourceCode);

	bytes expectation(bytes({byte(sof::Instruction::PUSH1), 0xbf}));
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_CASE(blockhash)
{
	char const* sourceCode = "contract test {\n"
							 "  function f() {\n"
							 "    block.blockhash(3);\n"
							 "  }\n"
							 "}\n";
	bytes code = compileFirstExpression(sourceCode, {}, {},
										{make_shared<MagicVariableDeclaration>("block", make_shared<MagicType>(MagicType::Kind::Block))});

	bytes expectation({byte(sof::Instruction::PUSH1), 0x03,
					   byte(sof::Instruction::BLOCKHASH)});
	BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(), code.end(), expectation.begin(), expectation.end());
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} // end namespaces
