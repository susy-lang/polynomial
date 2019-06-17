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
	std::map<const ContractDefinition*, sof::Assembly const*> const& _contracts
)
{
	ContractCompiler runtimeCompiler(m_runtimeContext, m_optimize);
	runtimeCompiler.compileContract(_contract, _contracts);

	ContractCompiler creationCompiler(m_context, m_optimize);
	m_runtimeSub = creationCompiler.compileConstructor(m_runtimeContext, _contract, _contracts);

	if (m_optimize)
		m_context.optimise(m_optimizeRuns);

	if (_contract.isLibrary())
	{
		polAssert(m_runtimeSub != size_t(-1), "");
		m_context.injectVersionStampIntoSub(m_runtimeSub);
	}
}

void Compiler::compileClone(
	ContractDefinition const& _contract,
	map<ContractDefinition const*, sof::Assembly const*> const& _contracts
)
{
	ContractCompiler cloneCompiler(m_context, m_optimize);
	m_runtimeSub = cloneCompiler.compileClone(_contract, _contracts);

	if (m_optimize)
		m_context.optimise(m_optimizeRuns);
}

sof::AssemblyItem Compiler::functionEntryLabel(FunctionDefinition const& _function) const
{
	return m_runtimeContext.functionEntryLabelIfExists(_function);
}
