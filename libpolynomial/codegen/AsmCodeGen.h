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

#pragma once

#include <libyul/AsmAnalysis.h>
#include <libyul/backends/svm/AbstractAssembly.h>
#include <liblangutil/SourceLocation.h>
#include <functional>

namespace yul
{
struct Block;
}

namespace dev
{
namespace sof
{
class Assembly;
class AssemblyItem;
}

namespace polynomial
{

class SofAssemblyAdapter: public yul::AbstractAssembly
{
public:
	explicit SofAssemblyAdapter(sof::Assembly& _assembly);
	void setSourceLocation(langutil::SourceLocation const& _location) override;
	int stackHeight() const override;
	void appendInstruction(polynomial::Instruction _instruction) override;
	void appendConstant(u256 const& _constant) override;
	void appendLabel(LabelID _labelId) override;
	void appendLabelReference(LabelID _labelId) override;
	size_t newLabelId() override;
	size_t namedLabel(std::string const& _name) override;
	void appendLinkerSymbol(std::string const& _linkerSymbol) override;
	void appendJump(int _stackDiffAfter) override;
	void appendJumpTo(LabelID _labelId, int _stackDiffAfter) override;
	void appendJumpToIf(LabelID _labelId) override;
	void appendBeginsub(LabelID, int) override;
	void appendJumpsub(LabelID, int, int) override;
	void appendReturnsub(int, int) override;
	void appendAssemblySize() override;
	std::pair<std::shared_ptr<AbstractAssembly>, SubID> createSubAssembly() override;
	void appendDataOffset(SubID _sub) override;
	void appendDataSize(SubID _sub) override;
	SubID appendData(dev::bytes const& _data) override;

private:
	static LabelID assemblyTagToIdentifier(sof::AssemblyItem const& _tag);

	sof::Assembly& m_assembly;
	std::map<SubID, dev::u256> m_dataHashBySubId;
	size_t m_nextDataCounter = std::numeric_limits<size_t>::max() / 2;
};

class CodeGenerator
{
public:
	/// Performs code generation and appends generated to _assembly.
	static void assemble(
		yul::Block const& _parsedData,
		yul::AsmAnalysisInfo& _analysisInfo,
		dev::sof::Assembly& _assembly,
		yul::ExternalIdentifierAccess const& _identifierAccess = yul::ExternalIdentifierAccess(),
		bool _useNamedLabelsForFunctions = false,
		bool _optimize = false
	);
};

}
}
