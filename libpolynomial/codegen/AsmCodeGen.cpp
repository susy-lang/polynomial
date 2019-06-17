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
 * Adaptor between the abstract assembly and sof assembly.
 */

#include <libpolynomial/codegen/AsmCodeGen.h>

#include <libyul/AsmData.h>
#include <libyul/AsmAnalysisInfo.h>

#include <libyul/backends/svm/AbstractAssembly.h>
#include <libyul/backends/svm/SVMCodeTransform.h>

#include <libsvmasm/Assembly.h>
#include <libsvmasm/AssemblyItem.h>
#include <libsvmasm/Instruction.h>

#include <liblangutil/SourceLocation.h>

#include <libdevcore/FixedHash.h>

#include <memory>
#include <functional>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace yul;
using namespace dev::polynomial;

SofAssemblyAdapter::SofAssemblyAdapter(sof::Assembly& _assembly):
	m_assembly(_assembly)
{
}

void SofAssemblyAdapter::setSourceLocation(SourceLocation const& _location)
{
	m_assembly.setSourceLocation(_location);
}

int SofAssemblyAdapter::stackHeight() const
{
	return m_assembly.deposit();
}

void SofAssemblyAdapter::appendInstruction(polynomial::Instruction _instruction)
{
	m_assembly.append(_instruction);
}

void SofAssemblyAdapter::appendConstant(u256 const& _constant)
{
	m_assembly.append(_constant);
}

void SofAssemblyAdapter::appendLabel(LabelID _labelId)
{
	m_assembly.append(sof::AssemblyItem(sof::Tag, _labelId));
}

void SofAssemblyAdapter::appendLabelReference(LabelID _labelId)
{
	m_assembly.append(sof::AssemblyItem(sof::PushTag, _labelId));
}

size_t SofAssemblyAdapter::newLabelId()
{
	return assemblyTagToIdentifier(m_assembly.newTag());
}

size_t SofAssemblyAdapter::namedLabel(std::string const& _name)
{
	return assemblyTagToIdentifier(m_assembly.namedTag(_name));
}

void SofAssemblyAdapter::appendLinkerSymbol(std::string const& _linkerSymbol)
{
	m_assembly.appendLibraryAddress(_linkerSymbol);
}

void SofAssemblyAdapter::appendJump(int _stackDiffAfter)
{
	appendInstruction(polynomial::Instruction::JUMP);
	m_assembly.adjustDeposit(_stackDiffAfter);
}

void SofAssemblyAdapter::appendJumpTo(LabelID _labelId, int _stackDiffAfter)
{
	appendLabelReference(_labelId);
	appendJump(_stackDiffAfter);
}

void SofAssemblyAdapter::appendJumpToIf(LabelID _labelId)
{
	appendLabelReference(_labelId);
	appendInstruction(polynomial::Instruction::JUMPI);
}

void SofAssemblyAdapter::appendBeginsub(LabelID, int)
{
	// TODO we could emulate that, though
	polAssert(false, "BEGINSUB not implemented for SVM 1.0");
}

void SofAssemblyAdapter::appendJumpsub(LabelID, int, int)
{
	// TODO we could emulate that, though
	polAssert(false, "JUMPSUB not implemented for SVM 1.0");
}

void SofAssemblyAdapter::appendReturnsub(int, int)
{
	// TODO we could emulate that, though
	polAssert(false, "RETURNSUB not implemented for SVM 1.0");
}

void SofAssemblyAdapter::appendAssemblySize()
{
	m_assembly.appendProgramSize();
}

pair<shared_ptr<AbstractAssembly>, AbstractAssembly::SubID> SofAssemblyAdapter::createSubAssembly()
{
	shared_ptr<sof::Assembly> assembly{make_shared<sof::Assembly>()};
	auto sub = m_assembly.newSub(assembly);
	return {make_shared<SofAssemblyAdapter>(*assembly), size_t(sub.data())};
}

void SofAssemblyAdapter::appendDataOffset(AbstractAssembly::SubID _sub)
{
	auto it = m_dataHashBySubId.find(_sub);
	if (it == m_dataHashBySubId.end())
		m_assembly.pushSubroutineOffset(size_t(_sub));
	else
		m_assembly << sof::AssemblyItem(sof::PushData, it->second);
}

void SofAssemblyAdapter::appendDataSize(AbstractAssembly::SubID _sub)
{
	auto it = m_dataHashBySubId.find(_sub);
	if (it == m_dataHashBySubId.end())
		m_assembly.pushSubroutineSize(size_t(_sub));
	else
		m_assembly << u256(m_assembly.data(h256(it->second)).size());
}

AbstractAssembly::SubID SofAssemblyAdapter::appendData(bytes const& _data)
{
	sof::AssemblyItem pushData = m_assembly.newData(_data);
	SubID subID = m_nextDataCounter++;
	m_dataHashBySubId[subID] = pushData.data();
	return subID;
}

SofAssemblyAdapter::LabelID SofAssemblyAdapter::assemblyTagToIdentifier(sof::AssemblyItem const& _tag)
{
	u256 id = _tag.data();
	polAssert(id <= std::numeric_limits<LabelID>::max(), "Tag id too large.");
	return LabelID(id);
}

void CodeGenerator::assemble(
	Block const& _parsedData,
	AsmAnalysisInfo& _analysisInfo,
	sof::Assembly& _assembly,
	ExternalIdentifierAccess const& _identifierAccess,
	bool _useNamedLabelsForFunctions,
	bool _optimize
)
{
	SofAssemblyAdapter assemblyAdapter(_assembly);
	shared_ptr<SVMDialect> dialect = SVMDialect::strictAssemblyForSVM();
	CodeTransform transform(
		assemblyAdapter,
		_analysisInfo,
		_parsedData,
		*dialect,
		_optimize,
		false,
		_identifierAccess,
		_useNamedLabelsForFunctions
	);
	try
	{
		transform(_parsedData);
	}
	catch (StackTooDeepError const& _e)
	{
		BOOST_THROW_EXCEPTION(
			InternalCompilerError() << errinfo_comment(
				"Stack too deep when compiling inline assembly" +
				(_e.comment() ? ": " + *_e.comment() : ".")
			));
	}
	polAssert(transform.stackErrors().empty(), "Stack errors present but not thrown.");
}
