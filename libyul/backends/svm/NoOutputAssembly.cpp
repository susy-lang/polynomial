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
 * Assembly interface that ignores everything. Can be used as a backend for a compilation dry-run.
 */

#include <libyul/backends/svm/NoOutputAssembly.h>

#include <libsvmasm/Instruction.h>

#include <liblangutil/Exceptions.h>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace yul;


void NoOutputAssembly::appendInstruction(polynomial::Instruction _instr)
{
	m_stackHeight += polynomial::instructionInfo(_instr).ret - polynomial::instructionInfo(_instr).args;
}

void NoOutputAssembly::appendConstant(u256 const&)
{
	appendInstruction(polynomial::pushInstruction(1));
}

void NoOutputAssembly::appendLabel(LabelID)
{
	appendInstruction(polynomial::Instruction::JUMPDEST);
}

void NoOutputAssembly::appendLabelReference(LabelID)
{
	polAssert(!m_svm15, "Cannot use plain label references in EMV1.5 mode.");
	appendInstruction(polynomial::pushInstruction(1));
}

NoOutputAssembly::LabelID NoOutputAssembly::newLabelId()
{
	return 1;
}

AbstractAssembly::LabelID NoOutputAssembly::namedLabel(string const&)
{
	return 1;
}

void NoOutputAssembly::appendLinkerSymbol(string const&)
{
	polAssert(false, "Linker symbols not yet implemented.");
}

void NoOutputAssembly::appendJump(int _stackDiffAfter)
{
	polAssert(!m_svm15, "Plain JUMP used for SVM 1.5");
	appendInstruction(polynomial::Instruction::JUMP);
	m_stackHeight += _stackDiffAfter;
}

void NoOutputAssembly::appendJumpTo(LabelID _labelId, int _stackDiffAfter)
{
	if (m_svm15)
		m_stackHeight += _stackDiffAfter;
	else
	{
		appendLabelReference(_labelId);
		appendJump(_stackDiffAfter);
	}
}

void NoOutputAssembly::appendJumpToIf(LabelID _labelId)
{
	if (m_svm15)
		m_stackHeight--;
	else
	{
		appendLabelReference(_labelId);
		appendInstruction(polynomial::Instruction::JUMPI);
	}
}

void NoOutputAssembly::appendBeginsub(LabelID, int _arguments)
{
	polAssert(m_svm15, "BEGINSUB used for SVM 1.0");
	polAssert(_arguments >= 0, "");
	m_stackHeight += _arguments;
}

void NoOutputAssembly::appendJumpsub(LabelID, int _arguments, int _returns)
{
	polAssert(m_svm15, "JUMPSUB used for SVM 1.0");
	polAssert(_arguments >= 0 && _returns >= 0, "");
	m_stackHeight += _returns - _arguments;
}

void NoOutputAssembly::appendReturnsub(int _returns, int _stackDiffAfter)
{
	polAssert(m_svm15, "RETURNSUB used for SVM 1.0");
	polAssert(_returns >= 0, "");
	m_stackHeight += _stackDiffAfter - _returns;
}

void NoOutputAssembly::appendAssemblySize()
{
	appendInstruction(polynomial::Instruction::PUSH1);
}

pair<shared_ptr<AbstractAssembly>, AbstractAssembly::SubID> NoOutputAssembly::createSubAssembly()
{
	polAssert(false, "Sub assemblies not implemented.");
	return {};
}

void NoOutputAssembly::appendDataOffset(AbstractAssembly::SubID)
{
	appendInstruction(polynomial::Instruction::PUSH1);
}

void NoOutputAssembly::appendDataSize(AbstractAssembly::SubID)
{
	appendInstruction(polynomial::Instruction::PUSH1);
}

AbstractAssembly::SubID NoOutputAssembly::appendData(bytes const&)
{
	return 1;
}

NoOutputSVMDialect::NoOutputSVMDialect(shared_ptr<SVMDialect> const& _copyFrom):
	SVMDialect(_copyFrom->flavour, _copyFrom->providesObjectAccess(), _copyFrom->svmVersion())
{
	for (auto& fun: m_functions)
	{
		size_t parameters = fun.second.parameters.size();
		size_t returns = fun.second.returns.size();
		fun.second.generateCode = [=](FunctionCall const&, AbstractAssembly& _assembly, std::function<void()> _visitArguments)
		{
			_visitArguments();
			for (size_t i = 0; i < parameters; i++)
				_assembly.appendInstruction(dev::polynomial::Instruction::POP);

			for (size_t i = 0; i < returns; i++)
				_assembly.appendConstant(u256(0));
		};
	}
}
