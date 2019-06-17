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
 * Polynomial AST to SVM bytecode compiler.
 */

#pragma once

#include <libpolynomial/codegen/CompilerContext.h>
#include <liblangutil/SVMVersion.h>

#include <libsvmasm/Assembly.h>

#include <ostream>
#include <functional>

namespace dev {
namespace polynomial {

class Compiler
{
public:
	explicit Compiler(SVMVersion _svmVersion = SVMVersion{}, bool _optimize = false, unsigned _runs = 200):
		m_optimize(_optimize),
		m_optimizeRuns(_runs),
		m_runtimeContext(_svmVersion),
		m_context(_svmVersion, &m_runtimeContext)
	{ }

	/// Compiles a contract.
	/// @arg _metadata contains the to be injected metadata CBOR
	void compileContract(
		ContractDefinition const& _contract,
		std::map<ContractDefinition const*, sof::Assembly const*> const& _contracts,
		bytes const& _metadata
	);
	/// @returns Entire assembly.
	sof::Assembly const& assembly() const { return m_context.assembly(); }
	/// @returns The entire assembled object (with constructor).
	sof::LinkerObject assembledObject() const { return m_context.assembledObject(); }
	/// @returns Only the runtime object (without constructor).
	sof::LinkerObject runtimeObject() const { return m_context.assembledRuntimeObject(m_runtimeSub); }
	/// @arg _sourceCodes is the map of input files to source code strings
	std::string assemblyString(StringMap const& _sourceCodes = StringMap()) const
	{
		return m_context.assemblyString(_sourceCodes);
	}
	/// @arg _sourceCodes is the map of input files to source code strings
	Json::Value assemblyJSON(StringMap const& _sourceCodes = StringMap()) const
	{
		return m_context.assemblyJSON(_sourceCodes);
	}
	/// @returns Assembly items of the normal compiler context
	sof::AssemblyItems const& assemblyItems() const { return m_context.assembly().items(); }
	/// @returns Assembly items of the runtime compiler context
	sof::AssemblyItems const& runtimeAssemblyItems() const { return m_context.assembly().sub(m_runtimeSub).items(); }

	/// @returns the entry label of the given function. Might return an AssemblyItem of type
	/// UndefinedItem if it does not exist yet.
	sof::AssemblyItem functionEntryLabel(FunctionDefinition const& _function) const;

private:
	bool const m_optimize;
	unsigned const m_optimizeRuns;
	CompilerContext m_runtimeContext;
	size_t m_runtimeSub = size_t(-1); ///< Identifier of the runtime sub-assembly, if present.
	CompilerContext m_context;
};

}
}
