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
 * @author Christian <c@sofdev.com>
 * @date 2016
 * Code-generating part of inline assembly.
 */

#include <libpolynomial/inlineasm/AsmCodeGen.h>

#include <libpolynomial/inlineasm/AsmParser.h>
#include <libpolynomial/inlineasm/AsmData.h>
#include <libpolynomial/inlineasm/AsmScope.h>
#include <libpolynomial/inlineasm/AsmAnalysis.h>
#include <libpolynomial/inlineasm/AsmAnalysisInfo.h>

#include <libsvmasm/Assembly.h>
#include <libsvmasm/SourceLocation.h>
#include <libsvmasm/Instruction.h>

#include <libjulia/backends/svm/AbstractAssembly.h>
#include <libjulia/backends/svm/SVMCodeTransform.h>

#include <libdevcore/CommonIO.h>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/count_if.hpp>

#include <memory>
#include <functional>

using namespace std;
using namespace dev;
using namespace dev::polynomial;
using namespace dev::polynomial::assembly;

class SofAssemblyAdapter: public julia::AbstractAssembly
{
public:
	SofAssemblyAdapter(sof::Assembly& _assembly):
		m_assembly(_assembly)
	{
	}
	virtual void setSourceLocation(SourceLocation const& _location) override
	{
		m_assembly.setSourceLocation(_location);
	}
	virtual int stackHeight() const override { return m_assembly.deposit(); }
	virtual void appendInstruction(polynomial::Instruction _instruction) override
	{
		m_assembly.append(_instruction);
	}
	virtual void appendConstant(u256 const& _constant) override
	{
		m_assembly.append(_constant);
	}
	/// Append a label.
	virtual void appendLabel(LabelID _labelId) override
	{
		m_assembly.append(sof::AssemblyItem(sof::Tag, _labelId));
	}
	/// Append a label reference.
	virtual void appendLabelReference(LabelID _labelId) override
	{
		m_assembly.append(sof::AssemblyItem(sof::PushTag, _labelId));
	}
	virtual size_t newLabelId() override
	{
		return assemblyTagToIdentifier(m_assembly.newTag());
	}
	virtual void appendLinkerSymbol(std::string const& _linkerSymbol) override
	{
		m_assembly.appendLibraryAddress(_linkerSymbol);
	}
	virtual void appendJump(int _stackDiffAfter) override
	{
		appendInstruction(polynomial::Instruction::JUMP);
		m_assembly.adjustDeposit(_stackDiffAfter);
	}
	virtual void appendJumpTo(LabelID _labelId, int _stackDiffAfter) override
	{
		appendLabelReference(_labelId);
		appendJump(_stackDiffAfter);
	}
	virtual void appendJumpToIf(LabelID _labelId) override
	{
		appendLabelReference(_labelId);
		appendInstruction(polynomial::Instruction::JUMPI);
	}
	virtual void appendBeginsub(LabelID, int) override
	{
		// TODO we could emulate that, though
		polAssert(false, "BEGINSUB not implemented for SVM 1.0");
	}
	/// Call a subroutine.
	virtual void appendJumpsub(LabelID, int, int) override
	{
		// TODO we could emulate that, though
		polAssert(false, "JUMPSUB not implemented for SVM 1.0");
	}

	/// Return from a subroutine.
	virtual void appendReturnsub(int, int) override
	{
		// TODO we could emulate that, though
		polAssert(false, "RETURNSUB not implemented for SVM 1.0");
	}

	virtual void appendAssemblySize() override
	{
		m_assembly.appendProgramSize();
	}

private:
	LabelID assemblyTagToIdentifier(sof::AssemblyItem const& _tag) const
	{
		u256 id = _tag.data();
		polAssert(id <= std::numeric_limits<LabelID>::max(), "Tag id too large.");
		return LabelID(id);
	}

	sof::Assembly& m_assembly;
};

void assembly::CodeGenerator::assemble(
	Block const& _parsedData,
	AsmAnalysisInfo& _analysisInfo,
	sof::Assembly& _assembly,
	julia::ExternalIdentifierAccess const& _identifierAccess
)
{
	SofAssemblyAdapter assemblyAdapter(_assembly);
	julia::CodeTransform(assemblyAdapter, _analysisInfo, false, false, _identifierAccess)(_parsedData);
}
