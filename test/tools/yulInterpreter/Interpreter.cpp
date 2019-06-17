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
 * Yul interpreter.
 */

#include <test/tools/yulInterpreter/Interpreter.h>

#include <test/tools/yulInterpreter/SVMInstructionInterpreter.h>

#include <libyul/AsmData.h>
#include <libyul/Utilities.h>

#include <liblangutil/Exceptions.h>

#include <libdevcore/FixedHash.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace yul;
using namespace yul::test;


void Interpreter::operator()(ExpressionStatement const& _expressionStatement)
{
	evaluateMulti(_expressionStatement.expression);
}

void Interpreter::operator()(Assignment const& _assignment)
{
	polAssert(_assignment.value, "");
	vector<u256> values = evaluateMulti(*_assignment.value);
	polAssert(values.size() == _assignment.variableNames.size(), "");
	for (size_t i = 0; i < values.size(); ++i)
	{
		YulString varName = _assignment.variableNames.at(i).name;
		polAssert(m_variables.count(varName), "");
		m_variables[varName] = values.at(i);
	}
}

void Interpreter::operator()(VariableDeclaration const& _declaration)
{
	vector<u256> values(_declaration.variables.size(), 0);
	if (_declaration.value)
		values = evaluateMulti(*_declaration.value);

	polAssert(values.size() == _declaration.variables.size(), "");
	for (size_t i = 0; i < values.size(); ++i)
	{
		YulString varName = _declaration.variables.at(i).name;
		polAssert(!m_variables.count(varName), "");
		m_variables[varName] = values.at(i);
		m_scopes.back().insert(varName);
	}
}

void Interpreter::operator()(If const& _if)
{
	polAssert(_if.condition, "");
	if (evaluate(*_if.condition) != 0)
		(*this)(_if.body);
}

void Interpreter::operator()(Switch const& _switch)
{
	polAssert(_switch.expression, "");
	u256 val = evaluate(*_switch.expression);
	polAssert(!_switch.cases.empty(), "");
	for (auto const& c: _switch.cases)
		// Default case has to be last.
		if (!c.value || evaluate(*c.value) == val)
		{
			(*this)(c.body);
			break;
		}
}

void Interpreter::operator()(FunctionDefinition const&)
{
}

void Interpreter::operator()(ForLoop const& _forLoop)
{
	polAssert(_forLoop.condition, "");

	openScope();
	for (auto const& statement: _forLoop.pre.statements)
		visit(statement);
	while (evaluate(*_forLoop.condition) != 0)
	{
		(*this)(_forLoop.body);
		(*this)(_forLoop.post);
	}
	closeScope();
}

void Interpreter::operator()(Block const& _block)
{
	openScope();
	// Register functions.
	for (auto const& statement: _block.statements)
		if (statement.type() == typeid(FunctionDefinition))
		{
			FunctionDefinition const& funDef = boost::get<FunctionDefinition>(statement);
			m_functions[funDef.name] = &funDef;
			m_scopes.back().insert(funDef.name);
		}
	ASTWalker::operator()(_block);
	closeScope();
}

u256 Interpreter::evaluate(Expression const& _expression)
{
	ExpressionEvaluator ev(m_state, m_variables, m_functions);
	ev.visit(_expression);
	return ev.value();
}

vector<u256> Interpreter::evaluateMulti(Expression const& _expression)
{
	ExpressionEvaluator ev(m_state, m_variables, m_functions);
	ev.visit(_expression);
	return ev.values();
}

void Interpreter::closeScope()
{
	for (auto const& var: m_scopes.back())
	{
		size_t erased = m_variables.erase(var) + m_functions.erase(var);
		polAssert(erased == 1, "");
	}
	m_scopes.pop_back();
}

void ExpressionEvaluator::operator()(Literal const& _literal)
{
	static YulString const trueString("true");
	static YulString const falseString("false");

	switch (_literal.kind)
	{
	case LiteralKind::Boolean:
		polAssert(_literal.value == trueString || _literal.value == falseString, "");
		setValue(_literal.value == trueString ? 1 : 0);
		break;
	case LiteralKind::Number:
		setValue(valueOfNumberLiteral(_literal));
		break;
	case LiteralKind::String:
		polAssert(_literal.value.str().size() <= 32, "");
		setValue(u256(h256(_literal.value.str(), h256::FromBinary, h256::AlignLeft)));
		break;
	}
}

void ExpressionEvaluator::operator()(Identifier const& _identifier)
{
	polAssert(m_variables.count(_identifier.name), "");
	setValue(m_variables.at(_identifier.name));
}

void ExpressionEvaluator::operator()(FunctionalInstruction const& _instr)
{
	evaluateArgs(_instr.arguments);
	SVMInstructionInterpreter interpreter(m_state);
	// The instruction might also return nothing, but it does not
	// hurt to set the value in that case.
	setValue(interpreter.eval(_instr.instruction, values()));
}

void ExpressionEvaluator::operator()(FunctionCall const& _funCall)
{
	polAssert(m_functions.count(_funCall.functionName.name), "");
	evaluateArgs(_funCall.arguments);

	FunctionDefinition const& fun = *m_functions.at(_funCall.functionName.name);
	polAssert(m_values.size() == fun.parameters.size(), "");
	map<YulString, u256> variables;
	for (size_t i = 0; i < fun.parameters.size(); ++i)
		variables[fun.parameters.at(i).name] = m_values.at(i);
	for (size_t i = 0; i < fun.returnVariables.size(); ++i)
		variables[fun.returnVariables.at(i).name] = 0;

	// TODO function name lookup could be a little more efficient,
	// we have to copy the list here.
	Interpreter interpreter(m_state, variables, m_functions);
	interpreter(fun.body);

	m_values.clear();
	for (auto const& retVar: fun.returnVariables)
		m_values.emplace_back(interpreter.valueOfVariable(retVar.name));
}

u256 ExpressionEvaluator::value() const
{
	polAssert(m_values.size() == 1, "");
	return m_values.front();
}

void ExpressionEvaluator::setValue(u256 _value)
{
	m_values.clear();
	m_values.emplace_back(std::move(_value));
}

void ExpressionEvaluator::evaluateArgs(vector<Expression> const& _expr)
{
	vector<u256> values;
	/// Function arguments are evaluated in reverse.
	for (auto const& expr: _expr | boost::adaptors::reversed)
	{
		visit(expr);
		values.push_back(value());
	}
	m_values = std::move(values);
	std::reverse(m_values.begin(), m_values.end());
}