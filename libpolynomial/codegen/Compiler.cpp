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
 * @date 2014
 * Polynomial compiler.
 */

#include <libpolynomial/codegen/Compiler.h>
#include <libsvmasm/Assembly.h>
#include <libpolynomial/codegen/ContractCompiler.h>

using namespace std;
using namespace dev;
using namespace dev::polynomial;

void Compiler::compileContract(
	ContractDefinition const& _contract,
	std::map<const ContractDefinition*, sof::Assembly const*> const& _contracts,
	bytes const& _metadata
)
{
	ContractCompiler runtimeCompiler(nullptr, m_runtimeContext, m_optimize);
	runtimeCompiler.compileContract(_contract, _contracts);
	m_runtimeContext.appendAuxiliaryData(_metadata);

	// This might modify m_runtimeContext because it can access runtime functions at
	// creation time.
	ContractCompiler creationCompiler(&runtimeCompiler, m_context, m_optimize);
	m_runtimeSub = creationCompiler.compileConstructor(_contract, _contracts);

	m_context.optimise(m_optimize, m_optimizeRuns);
}

void Compiler::compileClone(
	ContractDefinition const& _contract,
	map<ContractDefinition const*, sof::Assembly const*> const& _contracts
)
{
	ContractCompiler runtimeCompiler(nullptr, m_runtimeContext, m_optimize);
	ContractCompiler cloneCompiler(&runtimeCompiler, m_context, m_optimize);
	m_runtimeSub = cloneCompiler.compileClone(_contract, _contracts);

	m_context.optimise(m_optimize, m_optimizeRuns);
}

sof::AssemblyItem Compiler::functionEntryLabel(FunctionDefinition const& _function) const
{
	return m_runtimeContext.functionEntryLabelIfExists(_function);
}
