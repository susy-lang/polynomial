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
 * Assembly interface for SVM and SVM1.5.
 */

#include <libyul/backends/svm/SVMAssembly.h>

#include <libsvmasm/Instruction.h>

#include <liblangutil/Exceptions.h>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace yul;

namespace
{
/// Size of labels in bytes. Four-byte labels are required by some SVM1.5 instructions.
size_t constexpr labelReferenceSize = 4;

size_t constexpr assemblySizeReferenceSize = 4;
}


void SVMAssembly::setSourceLocation(SourceLocation const&)
{
	// Ignored for now;
}

void SVMAssembly::appendInstruction(polynomial::Instruction _instr)
{
	m_bytecode.push_back(uint8_t(_instr));
	m_stackHeight += polynomial::instructionInfo(_instr).ret - polynomial::instructionInfo(_instr).args;
}

void SVMAssembly::appendConstant(u256 const& _constant)
{
	bytes data = toCompactBigEndian(_constant, 1);
	appendInstruction(polynomial::pushInstruction(data.size()));
	m_bytecode += data;
}

void SVMAssembly::appendLabel(LabelID _labelId)
{
	setLabelToCurrentPosition(_labelId);
	appendInstruction(polynomial::Instruction::JUMPDEST);
}

void SVMAssembly::appendLabelReference(LabelID _labelId)
{
	polAssert(!m_svm15, "Cannot use plain label references in EMV1.5 mode.");
	// @TODO we now always use labelReferenceSize for all labels, it could be shortened
	// for some of them.
	appendInstruction(polynomial::pushInstruction(labelReferenceSize));
	m_labelReferences[m_bytecode.size()] = _labelId;
	m_bytecode += bytes(labelReferenceSize);
}

SVMAssembly::LabelID SVMAssembly::newLabelId()
{
	m_labelPositions[m_nextLabelId] = size_t(-1);
	return m_nextLabelId++;
}

AbstractAssembly::LabelID SVMAssembly::namedLabel(string const& _name)
{
	polAssert(!_name.empty(), "");
	if (!m_namedLabels.count(_name))
		m_namedLabels[_name] = newLabelId();
	return m_namedLabels[_name];
}

void SVMAssembly::appendLinkerSymbol(string const&)
{
	polAssert(false, "Linker symbols not yet implemented.");
}

void SVMAssembly::appendJump(int _stackDiffAfter)
{
	polAssert(!m_svm15, "Plain JUMP used for SVM 1.5");
	appendInstruction(polynomial::Instruction::JUMP);
	m_stackHeight += _stackDiffAfter;
}

void SVMAssembly::appendJumpTo(LabelID _labelId, int _stackDiffAfter)
{
	if (m_svm15)
	{
		m_bytecode.push_back(uint8_t(polynomial::Instruction::JUMPTO));
		appendLabelReferenceInternal(_labelId);
		m_stackHeight += _stackDiffAfter;
	}
	else
	{
		appendLabelReference(_labelId);
		appendJump(_stackDiffAfter);
	}
}

void SVMAssembly::appendJumpToIf(LabelID _labelId)
{
	if (m_svm15)
	{
		m_bytecode.push_back(uint8_t(polynomial::Instruction::JUMPIF));
		appendLabelReferenceInternal(_labelId);
		m_stackHeight--;
	}
	else
	{
		appendLabelReference(_labelId);
		appendInstruction(polynomial::Instruction::JUMPI);
	}
}

void SVMAssembly::appendBeginsub(LabelID _labelId, int _arguments)
{
	polAssert(m_svm15, "BEGINSUB used for SVM 1.0");
	polAssert(_arguments >= 0, "");
	setLabelToCurrentPosition(_labelId);
	m_bytecode.push_back(uint8_t(polynomial::Instruction::BEGINSUB));
	m_stackHeight += _arguments;
}

void SVMAssembly::appendJumpsub(LabelID _labelId, int _arguments, int _returns)
{
	polAssert(m_svm15, "JUMPSUB used for SVM 1.0");
	polAssert(_arguments >= 0 && _returns >= 0, "");
	m_bytecode.push_back(uint8_t(polynomial::Instruction::JUMPSUB));
	appendLabelReferenceInternal(_labelId);
	m_stackHeight += _returns - _arguments;
}

void SVMAssembly::appendReturnsub(int _returns, int _stackDiffAfter)
{
	polAssert(m_svm15, "RETURNSUB used for SVM 1.0");
	polAssert(_returns >= 0, "");
	m_bytecode.push_back(uint8_t(polynomial::Instruction::RETURNSUB));
	m_stackHeight += _stackDiffAfter - _returns;
}

sof::LinkerObject SVMAssembly::finalize()
{
	size_t bytecodeSize = m_bytecode.size();
	for (auto const& ref: m_assemblySizePositions)
		updateReference(ref, assemblySizeReferenceSize, u256(bytecodeSize));

	for (auto const& ref: m_labelReferences)
	{
		size_t referencePos = ref.first;
		polAssert(m_labelPositions.count(ref.second), "");
		size_t labelPos = m_labelPositions.at(ref.second);
		polAssert(labelPos != size_t(-1), "Undefined but allocated label used.");
		updateReference(referencePos, labelReferenceSize, u256(labelPos));
	}

	sof::LinkerObject obj;
	obj.bytecode = m_bytecode;
	return obj;
}

void SVMAssembly::setLabelToCurrentPosition(LabelID _labelId)
{
	polAssert(m_labelPositions.count(_labelId), "Label not found.");
	polAssert(m_labelPositions[_labelId] == size_t(-1), "Label already set.");
	m_labelPositions[_labelId] = m_bytecode.size();
}

void SVMAssembly::appendLabelReferenceInternal(LabelID _labelId)
{
	m_labelReferences[m_bytecode.size()] = _labelId;
	m_bytecode += bytes(labelReferenceSize);
}

void SVMAssembly::appendAssemblySize()
{
	appendInstruction(polynomial::pushInstruction(assemblySizeReferenceSize));
	m_assemblySizePositions.push_back(m_bytecode.size());
	m_bytecode += bytes(assemblySizeReferenceSize);
}

pair<shared_ptr<AbstractAssembly>, AbstractAssembly::SubID> SVMAssembly::createSubAssembly()
{
	polAssert(false, "Sub assemblies not implemented.");
	return {};
}

void SVMAssembly::appendDataOffset(AbstractAssembly::SubID)
{
	polAssert(false, "Data not implemented.");
}

void SVMAssembly::appendDataSize(AbstractAssembly::SubID)
{
	polAssert(false, "Data not implemented.");
}

AbstractAssembly::SubID SVMAssembly::appendData(bytes const&)
{
	polAssert(false, "Data not implemented.");
}

void SVMAssembly::updateReference(size_t pos, size_t size, u256 value)
{
	polAssert(m_bytecode.size() >= size && pos <= m_bytecode.size() - size, "");
	polAssert(value < (u256(1) << (8 * size)), "");
	for (size_t i = 0; i < size; i++)
		m_bytecode[pos + i] = uint8_t((value >> (8 * (size - i - 1))) & 0xff);
}
