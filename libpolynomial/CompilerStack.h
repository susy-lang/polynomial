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
 * @author Gav Wood <g@sofdev.com>
 * @date 2014
 * Full-stack compiler that converts a source code string to bytecode.
 */

#pragma once

#include <ostream>
#include <string>
#include <memory>
#include <vector>
#include <boost/noncopyable.hpp>
#include <json/json.h>
#include <libdevcore/Common.h>
#include <libdevcore/FixedHash.h>
#include <libsvmasm/SourceLocation.h>
#include <libsvmasm/LinkerObject.h>

namespace dev
{

namespace sof
{
class AssemblyItem;
using AssemblyItems = std::vector<AssemblyItem>;
}

namespace polynomial
{

// forward declarations
class Scanner;
class ContractDefinition;
class FunctionDefinition;
class SourceUnit;
class Compiler;
class GlobalContext;
class InterfaceHandler;

enum class DocumentationType: uint8_t
{
	NatspecUser = 1,
	NatspecDev,
	ABIInterface,
	ABIPolynomialInterface
};

/**
 * Easy to use and self-contained Polynomial compiler with as few header dependencies as possible.
 * It holds state and can be used to either step through the compilation stages (and abort e.g.
 * before compilation to bytecode) or run the whole compilation in one call.
 */
class CompilerStack: boost::noncopyable
{
public:
	/// Creates a new compiler stack. Adds standard sources if @a _addStandardSources.
	explicit CompilerStack(bool _addStandardSources = true);

	/// Resets the compiler to a state where the sources are not parsed or even removed.
	void reset(bool _keepSources = false, bool _addStandardSources = true);

	/// Adds a source object (e.g. file) to the parser. After this, parse has to be called again.
	/// @returns true if a source object by the name already existed and was replaced.
	void addSources(StringMap const& _nameContents, bool _isLibrary = false) { for (auto const& i: _nameContents) addSource(i.first, i.second, _isLibrary); }
	bool addSource(std::string const& _name, std::string const& _content, bool _isLibrary = false);
	void setSource(std::string const& _sourceCode);
	/// Parses all source units that were added
	void parse();
	/// Sets the given source code as the only source unit apart from standard sources and parses it.
	void parse(std::string const& _sourceCode);
	/// Returns a list of the contract names in the sources.
	std::vector<std::string> contractNames() const;
	std::string defaultContractName() const;

	/// Compiles the source units that were previously added and parsed.
	void compile(bool _optimize = false, unsigned _runs = 200);
	/// Parses and compiles the given source code.
	/// @returns the compiled linker object
	sof::LinkerObject const& compile(std::string const& _sourceCode, bool _optimize = false);

	/// Inserts the given addresses into the linker objects of all compiled contracts.
	void link(std::map<std::string, h160> const& _libraries);

	/// @returns the assembled object for a contract.
	sof::LinkerObject const& object(std::string const& _contractName = "") const;
	/// @returns the runtime object for the contract.
	sof::LinkerObject const& runtimeObject(std::string const& _contractName = "") const;
	/// @returns the bytecode of a contract that uses an already deployed contract via CALLCODE.
	/// The returned bytes will contain a sequence of 20 bytes of the format "XXX...XXX" which have to
	/// substituted by the actual address. Note that this sequence starts end ends in three X
	/// characters but can contain anything in between.
	sof::LinkerObject const& cloneObject(std::string const& _contractName = "") const;
	/// @returns normal contract assembly items
	sof::AssemblyItems const* assemblyItems(std::string const& _contractName = "") const;
	/// @returns runtime contract assembly items
	sof::AssemblyItems const* runtimeAssemblyItems(std::string const& _contractName = "") const;
	/// @returns hash of the runtime bytecode for the contract, i.e. the code that is
	/// returned by the constructor or the zero-h256 if the contract still needs to be linked or
	/// does not have runtime code.
	dev::h256 contractCodeHash(std::string const& _contractName = "") const;

	/// Streams a verbose version of the assembly to @a _outStream.
	/// @arg _sourceCodes is the map of input files to source code strings
	/// @arg _inJsonFromat shows whether the out should be in Json format
	/// Prerequisite: Successful compilation.
	Json::Value streamAssembly(std::ostream& _outStream, std::string const& _contractName = "", StringMap _sourceCodes = StringMap(), bool _inJsonFormat = false) const;

	/// Returns a string representing the contract interface in JSON.
	/// Prerequisite: Successful call to parse or compile.
	std::string const& interface(std::string const& _contractName = "") const;
	/// Returns a string representing the contract interface in Polynomial.
	/// Prerequisite: Successful call to parse or compile.
	std::string const& polynomialInterface(std::string const& _contractName = "") const;
	/// Returns a string representing the contract's documentation in JSON.
	/// Prerequisite: Successful call to parse or compile.
	/// @param type The type of the documentation to get.
	/// Can be one of 4 types defined at @c DocumentationType
	std::string const& metadata(std::string const& _contractName, DocumentationType _type) const;

	/// @returns the previously used scanner, useful for counting lines during error reporting.
	Scanner const& scanner(std::string const& _sourceName = "") const;
	/// @returns the parsed source unit with the supplied name.
	SourceUnit const& ast(std::string const& _sourceName = "") const;
	/// @returns the parsed contract with the supplied name. Throws an exception if the contract
	/// does not exist.
	ContractDefinition const& contractDefinition(std::string const& _contractName) const;

	/// @returns the offset of the entry point of the given function into the list of assembly items
	/// or zero if it is not found or does not exist.
	size_t functionEntryPoint(
		std::string const& _contractName,
		FunctionDefinition const& _function
	) const;

	/// Compile the given @a _sourceCode to bytecode. If a scanner is provided, it is used for
	/// scanning the source code - this is useful for printing exception information.
	static sof::LinkerObject staticCompile(std::string const& _sourceCode, bool _optimize = false);

	/// Helper function for logs printing. Do only use in error cases, it's quite expensive.
	/// line and columns are numbered starting from 1 with following order:
	/// start line, start column, end line, end column
	std::tuple<int, int, int, int> positionFromSourceLocation(SourceLocation const& _sourceLocation) const;

private:
	/**
	 * Information pertaining to one source unit, filled gradually during parsing and compilation.
	 */
	struct Source
	{
		std::shared_ptr<Scanner> scanner;
		std::shared_ptr<SourceUnit> ast;
		std::string interface;
		bool isLibrary = false;
		void reset() { scanner.reset(); ast.reset(); interface.clear(); }
	};

	struct Contract
	{
		ContractDefinition const* contract = nullptr;
		std::shared_ptr<Compiler> compiler;
		sof::LinkerObject object;
		sof::LinkerObject runtimeObject;
		sof::LinkerObject cloneObject;
		std::shared_ptr<InterfaceHandler> interfaceHandler;
		mutable std::unique_ptr<std::string const> interface;
		mutable std::unique_ptr<std::string const> polynomialInterface;
		mutable std::unique_ptr<std::string const> userDocumentation;
		mutable std::unique_ptr<std::string const> devDocumentation;

		Contract();
	};

	void resolveImports();

	Contract const& contract(std::string const& _contractName = "") const;
	Source const& source(std::string const& _sourceName = "") const;

	bool m_parseSuccessful;
	std::map<std::string const, Source> m_sources;
	std::shared_ptr<GlobalContext> m_globalContext;
	std::vector<Source const*> m_sourceOrder;
	std::map<std::string const, Contract> m_contracts;
};

}
}
