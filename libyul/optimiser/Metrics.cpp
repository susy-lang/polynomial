/*(
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
* Module providing metrics for the optimizer.
*/

#include <libyul/optimiser/Metrics.h>

#include <libyul/AsmData.h>
#include <libyul/Exceptions.h>
#include <libyul/Utilities.h>
#include <libyul/backends/svm/SVMDialect.h>

#include <libsvmasm/Instruction.h>
#include <libsvmasm/GasMeter.h>

#include <libdevcore/Visitor.h>
#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;

size_t CodeSize::codeSize(Statement const& _statement)
{
	CodeSize cs;
	cs.visit(_statement);
	return cs.m_size;
}

size_t CodeSize::codeSize(Expression const& _expression)
{
	CodeSize cs;
	cs.visit(_expression);
	return cs.m_size;
}

size_t CodeSize::codeSize(Block const& _block)
{
	CodeSize cs;
	cs(_block);
	return cs.m_size;
}

size_t CodeSize::codeSizeIncludingFunctions(Block const& _block)
{
	CodeSize cs(false);
	cs(_block);
	return cs.m_size;
}

void CodeSize::visit(Statement const& _statement)
{
	if (_statement.type() == typeid(FunctionDefinition) && m_ignoreFunctions)
		return;
	else if (
		_statement.type() == typeid(If) ||
		_statement.type() == typeid(Break) ||
		_statement.type() == typeid(Continue)
	)
		m_size += 2;
	else if (_statement.type() == typeid(ForLoop))
		m_size += 3;
	else if (_statement.type() == typeid(Switch))
		m_size += 1 + 2 * boost::get<Switch>(_statement).cases.size();
	else if (!(
		_statement.type() == typeid(Block) ||
		_statement.type() == typeid(ExpressionStatement) ||
		_statement.type() == typeid(Assignment) ||
		_statement.type() == typeid(VariableDeclaration)
	))
		++m_size;

	ASTWalker::visit(_statement);
}

void CodeSize::visit(Expression const& _expression)
{
	if (_expression.type() != typeid(Identifier))
		++m_size;
	ASTWalker::visit(_expression);
}


size_t CodeCost::codeCost(Dialect const& _dialect, Expression const& _expr)
{
	CodeCost cc(_dialect);
	cc.visit(_expr);
	return cc.m_cost;
}


void CodeCost::operator()(FunctionCall const& _funCall)
{
	ASTWalker::operator()(_funCall);

	if (SVMDialect const* dialect = dynamic_cast<SVMDialect const*>(&m_dialect))
		if (BuiltinFunctionForSVM const* f = dialect->builtin(_funCall.functionName.name))
			if (f->instruction)
			{
				addInstructionCost(*f->instruction);
				return;
			}

	m_cost += 49;
}

void CodeCost::operator()(FunctionalInstruction const& _instr)
{
	yulAssert(m_cost >= 1, "Should assign cost one in visit(Expression).");
	addInstructionCost(_instr.instruction);
	ASTWalker::operator()(_instr);
}

void CodeCost::operator()(Literal const& _literal)
{
	yulAssert(m_cost >= 1, "Should assign cost one in visit(Expression).");
	size_t cost = 0;
	switch (_literal.kind)
	{
	case LiteralKind::Boolean:
		break;
	case LiteralKind::Number:
		for (u256 n = u256(_literal.value.str()); n >= 0x100; n >>= 8)
			cost++;
		break;
	case LiteralKind::String:
		cost = _literal.value.str().size();
		break;
	}

	m_cost += cost;
}

void CodeCost::visit(Statement const& _statement)
{
	++m_cost;
	ASTWalker::visit(_statement);
}

void CodeCost::visit(Expression const& _expression)
{
	++m_cost;
	ASTWalker::visit(_expression);
}

void CodeCost::addInstructionCost(sof::Instruction _instruction)
{
	dev::sof::Tier gasPriceTier = dev::sof::instructionInfo(_instruction).gasPriceTier;
	if (gasPriceTier < dev::sof::Tier::VeryLow)
		m_cost -= 1;
	else if (gasPriceTier < dev::sof::Tier::High)
		m_cost += 1;
	else
		m_cost += 49;
}

size_t GasMeter::costs(Expression const& _expression) const
{
	return combineCosts(GasMeterVisitor::costs(_expression, m_dialect, m_isCreation));
}

size_t GasMeter::instructionCosts(sof::Instruction _instruction) const
{
	return combineCosts(GasMeterVisitor::instructionCosts(_instruction, m_dialect, m_isCreation));
}

size_t GasMeter::combineCosts(std::pair<size_t, size_t> _costs) const
{
	return _costs.first * m_runs + _costs.second;
}


pair<size_t, size_t> GasMeterVisitor::costs(
	Expression const& _expression,
	SVMDialect const& _dialect,
	bool _isCreation
)
{
	GasMeterVisitor gmv(_dialect, _isCreation);
	gmv.visit(_expression);
	return {gmv.m_runGas, gmv.m_dataGas};
}

pair<size_t, size_t> GasMeterVisitor::instructionCosts(
	dev::sof::Instruction _instruction,
	SVMDialect const& _dialect,
	bool _isCreation
)
{
	GasMeterVisitor gmv(_dialect, _isCreation);
	gmv.instructionCostsInternal(_instruction);
	return {gmv.m_runGas, gmv.m_dataGas};
}

void GasMeterVisitor::operator()(FunctionCall const& _funCall)
{
	ASTWalker::operator()(_funCall);
	if (BuiltinFunctionForSVM const* f = m_dialect.builtin(_funCall.functionName.name))
		if (f->instruction)
		{
			instructionCostsInternal(*f->instruction);
			return;
		}
	yulAssert(false, "Functions not implemented.");
}

void GasMeterVisitor::operator()(FunctionalInstruction const& _fun)
{
	ASTWalker::operator()(_fun);
	instructionCostsInternal(_fun.instruction);
}

void GasMeterVisitor::operator()(Literal const& _lit)
{
	m_runGas += dev::sof::GasMeter::runGas(dev::sof::Instruction::PUSH1);
	m_dataGas +=
		singleByteDataGas() +
		size_t(dev::sof::GasMeter::dataGas(dev::toCompactBigEndian(valueOfLiteral(_lit), 1), m_isCreation));
}

void GasMeterVisitor::operator()(Identifier const&)
{
	m_runGas += dev::sof::GasMeter::runGas(dev::sof::Instruction::DUP1);
	m_dataGas += singleByteDataGas();
}

size_t GasMeterVisitor::singleByteDataGas() const
{
	if (m_isCreation)
		return dev::sof::GasCosts::txDataNonZeroGas;
	else
		return dev::sof::GasCosts::createDataGas;
}

void GasMeterVisitor::instructionCostsInternal(dev::sof::Instruction _instruction)
{
	if (_instruction == sof::Instruction::EXP)
		m_runGas += dev::sof::GasCosts::expGas + dev::sof::GasCosts::expByteGas(m_dialect.svmVersion());
	else
		m_runGas += dev::sof::GasMeter::runGas(_instruction);
	m_dataGas += singleByteDataGas();
}

void AssignmentCounter::operator()(Assignment const& _assignment)
{
	for (auto const& variable: _assignment.variableNames)
		++m_assignmentCounters[variable.name];
}

size_t AssignmentCounter::assignmentCount(YulString _name) const
{
	auto it = m_assignmentCounters.find(_name);
	return (it == m_assignmentCounters.end()) ? 0 : it->second;
}
