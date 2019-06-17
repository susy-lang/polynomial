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
 * @author Gav Wood <g@sofdev.com>
 * @date 2014
 * Full-stack compiler that converts a source code string to bytecode.
 */

#pragma once

#include <libpolynomial/interface/ReadFile.h>
#include <libpolynomial/interface/OptimiserSettings.h>

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/SVMVersion.h>
#include <liblangutil/SourceLocation.h>

#include <libsvmasm/LinkerObject.h>

#include <libdevcore/Common.h>
#include <libdevcore/FixedHash.h>

#include <boost/noncopyable.hpp>
#include <json/json.h>

#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace langutil
{
class Scanner;
}

namespace dev
{

namespace sof
{
class Assembly;
class AssemblyItem;
using AssemblyItems = std::vector<AssemblyItem>;
}

namespace polynomial
{

// forward declarations
class ASTNode;
class ContractDefinition;
class FunctionDefinition;
class SourceUnit;
class Compiler;
class GlobalContext;
class Natspec;
class DeclarationContainer;

/**
 * Easy to use and self-contained Polynomial compiler with as few header dependencies as possible.
 * It holds state and can be used to either step through the compilation stages (and abort e.g.
 * before compilation to bytecode) or run the whole compilation in one call.
 */
class CompilerStack: boost::noncopyable
{
public:
	enum State {
		Empty,
		SourcesSet,
		ParsingSuccessful,
		AnalysisSuccessful,
		CompilationSuccessful
	};

	struct Remapping
	{
		std::string context;
		std::string prefix;
		std::string target;
	};

	/// Creates a new compiler stack.
	/// @param _readFile callback to used to read files for import statements. Must return
	/// and must not emit exceptions.
	explicit CompilerStack(ReadCallback::Callback const& _readFile = ReadCallback::Callback()):
		m_readFile(_readFile),
		m_errorList(),
		m_errorReporter(m_errorList) {}

	/// @returns the list of errors that occurred during parsing and type checking.
	langutil::ErrorList const& errors() const { return m_errorReporter.errors(); }

	/// @returns the current state.
	State state() const { return m_stackState; }

	/// Resets the compiler to a state where the sources are not parsed or even removed.
	/// Sets the state to SourcesSet if @a _keepSources is true, otherwise to Empty.
	/// All settings, with the exception of remappings, are reset.
	void reset(bool _keepSources = false);

	// Parses a remapping of the format "context:prefix=target".
	static boost::optional<Remapping> parseRemapping(std::string const& _remapping);

	/// Sets path remappings.
	/// Must be set before parsing.
	void setRemappings(std::vector<Remapping> const& _remappings);

	/// Sets library addresses. Addresses are cleared iff @a _libraries is missing.
	/// Must be set before parsing.
	void setLibraries(std::map<std::string, h160> const& _libraries = std::map<std::string, h160>{});

	/// Changes the optimiser settings.
	/// Must be set before parsing.
	void setOptimiserSettings(bool _optimize, unsigned _runs = 200);

	/// Changes the optimiser settings.
	/// Must be set before parsing.
	void setOptimiserSettings(OptimiserSettings _settings);

	/// Set the SVM version used before running compile.
	/// When called without an argument it will revert to the default version.
	/// Must be set before parsing.
	void setSVMVersion(langutil::SVMVersion _version = langutil::SVMVersion{});

	/// Sets the list of requested contract names. If empty, no filtering is performed and every contract
	/// found in the supplied sources is compiled. Names are cleared iff @a _contractNames is missing.
	void setRequestedContractNames(std::set<std::string> const& _contractNames = std::set<std::string>{}) {
		m_requestedContractNames = _contractNames;
	}

	/// @arg _metadataLiteralSources When true, store sources as literals in the contract metadata.
	/// Must be set before parsing.
	void useMetadataLiteralSources(bool _metadataLiteralSources);

	/// Adds a source object (e.g. file) to the parser. After this, parse has to be called again.
	/// @returns true if a source object by the name already existed and was replaced.
	bool addSource(std::string const& _name, std::string const& _content);

	/// Adds a response to an SMTLib2 query (identified by the hash of the query input).
	/// Must be set before parsing.
	void addSMTLib2Response(h256 const& _hash, std::string const& _response);

	/// Parses all source units that were added
	/// @returns false on error.
	bool parse();

	/// Performs the analysis steps (imports, scopesetting, syntaxCheck, referenceRepolving,
	///  typechecking, staticAnalysis) on previously parsed sources.
	/// @returns false on error.
	bool analyze();

	/// Parses and analyzes all source units that were added
	/// @returns false on error.
	bool parseAndAnalyze();

	/// Compiles the source units that were previously added and parsed.
	/// @returns false on error.
	bool compile();

	/// @returns the list of sources (paths) used
	std::vector<std::string> sourceNames() const;

	/// @returns a mapping assigning each source name its index inside the vector returned
	/// by sourceNames().
	std::map<std::string, unsigned> sourceIndices() const;

	/// @returns the previously used scanner, useful for counting lines during error reporting.
	langutil::Scanner const& scanner(std::string const& _sourceName) const;

	/// @returns the parsed source unit with the supplied name.
	SourceUnit const& ast(std::string const& _sourceName) const;

	/// Helper function for logs printing. Do only use in error cases, it's quite expensive.
	/// line and columns are numbered starting from 1 with following order:
	/// start line, start column, end line, end column
	std::tuple<int, int, int, int> positionFromSourceLocation(langutil::SourceLocation const& _sourceLocation) const;

	/// @returns a list of unhandled queries to the SMT solver (has to be supplied in a second run
	/// by calling @a addSMTLib2Response).
	std::vector<std::string> const& unhandledSMTLib2Queries() const { return m_unhandledSMTLib2Queries; }

	/// @returns a list of the contract names in the sources.
	std::vector<std::string> contractNames() const;

	/// @returns the name of the last contract.
	std::string const lastContractName() const;

	/// @returns either the contract's name or a mixture of its name and source file, sanitized for filesystem use
	std::string const filesystemFriendlyName(std::string const& _contractName) const;

	/// @returns the assembled object for a contract.
	sof::LinkerObject const& object(std::string const& _contractName) const;

	/// @returns the runtime object for the contract.
	sof::LinkerObject const& runtimeObject(std::string const& _contractName) const;

	/// @returns normal contract assembly items
	sof::AssemblyItems const* assemblyItems(std::string const& _contractName) const;

	/// @returns runtime contract assembly items
	sof::AssemblyItems const* runtimeAssemblyItems(std::string const& _contractName) const;

	/// @returns the string that provides a mapping between bytecode and sourcecode or a nullptr
	/// if the contract does not (yet) have bytecode.
	std::string const* sourceMapping(std::string const& _contractName) const;

	/// @returns the string that provides a mapping between runtime bytecode and sourcecode.
	/// if the contract does not (yet) have bytecode.
	std::string const* runtimeSourceMapping(std::string const& _contractName) const;

	/// @return a verbose text representation of the assembly.
	/// @arg _sourceCodes is the map of input files to source code strings
	/// Prerequisite: Successful compilation.
	std::string assemblyString(std::string const& _contractName, StringMap _sourceCodes = StringMap()) const;

	/// @returns a JSON representation of the assembly.
	/// @arg _sourceCodes is the map of input files to source code strings
	/// Prerequisite: Successful compilation.
	Json::Value assemblyJSON(std::string const& _contractName, StringMap _sourceCodes = StringMap()) const;

	/// @returns a JSON representing the contract ABI.
	/// Prerequisite: Successful call to parse or compile.
	Json::Value const& contractABI(std::string const& _contractName) const;

	/// @returns a JSON representing the contract's user documentation.
	/// Prerequisite: Successful call to parse or compile.
	Json::Value const& natspecUser(std::string const& _contractName) const;

	/// @returns a JSON representing the contract's developer documentation.
	/// Prerequisite: Successful call to parse or compile.
	Json::Value const& natspecDev(std::string const& _contractName) const;

	/// @returns a JSON representing a map of method identifiers (hashes) to function names.
	Json::Value methodIdentifiers(std::string const& _contractName) const;

	/// @returns the Contract Metadata
	std::string const& metadata(std::string const& _contractName) const;

	/// @returns a JSON representing the estimated gas usage for contract creation, internal and external functions
	Json::Value gasEstimates(std::string const& _contractName) const;

private:
	/// The state per source unit. Filled gradually during parsing.
	struct Source
	{
		std::shared_ptr<langutil::Scanner> scanner;
		std::shared_ptr<SourceUnit> ast;
		h256 mutable keccak256HashCached;
		h256 mutable swarmHashCached;
		void reset() { *this = Source(); }
		h256 const& keccak256() const;
		h256 const& swarmHash() const;
	};

	/// The state per contract. Filled gradually during compilation.
	struct Contract
	{
		ContractDefinition const* contract = nullptr;
		std::shared_ptr<Compiler> compiler;
		sof::LinkerObject object; ///< Deployment object (includes the runtime sub-object).
		sof::LinkerObject runtimeObject; ///< Runtime object.
		mutable std::unique_ptr<std::string const> metadata; ///< The metadata json that will be hashed into the chain.
		mutable std::unique_ptr<Json::Value const> abi;
		mutable std::unique_ptr<Json::Value const> userDocumentation;
		mutable std::unique_ptr<Json::Value const> devDocumentation;
		mutable std::unique_ptr<std::string const> sourceMapping;
		mutable std::unique_ptr<std::string const> runtimeSourceMapping;
	};

	/// Loads the missing sources from @a _ast (named @a _path) using the callback
	/// @a m_readFile and stores the absolute paths of all imports in the AST annotations.
	/// @returns the newly loaded sources.
	StringMap loadMissingSources(SourceUnit const& _ast, std::string const& _path);
	std::string applyRemapping(std::string const& _path, std::string const& _context);
	void resolveImports();

	/// @returns true if the contract is requested to be compiled.
	bool isRequestedContract(ContractDefinition const& _contract) const;

	/// Compile a single contract.
	/// @param _otherCompilers provides access to compilers of other contracts, to get
	///                        their bytecode if needed. Only filled after they have been compiled.
	void compileContract(
		ContractDefinition const& _contract,
		std::map<ContractDefinition const*, std::shared_ptr<Compiler const>>& _otherCompilers
	);

	/// Links all the known library addresses in the available objects. Any unknown
	/// library will still be kept as an unlinked placeholder in the objects.
	void link();

	/// @returns the contract object for the given @a _contractName.
	/// Can only be called after state is CompilationSuccessful.
	Contract const& contract(std::string const& _contractName) const;

	/// @returns the source object for the given @a _sourceName.
	/// Can only be called after state is SourcesSet.
	Source const& source(std::string const& _sourceName) const;

	/// @returns the parsed contract with the supplied name. Throws an exception if the contract
	/// does not exist.
	ContractDefinition const& contractDefinition(std::string const& _contractName) const;

	/// @returns the metadata JSON as a compact string for the given contract.
	std::string createMetadata(Contract const& _contract) const;

	/// @returns the metadata CBOR for the given serialised metadata JSON.
	static bytes createCBORMetadata(std::string const& _metadata, bool _experimentalMode);

	/// @returns the computer source mapping string.
	std::string computeSourceMapping(sof::AssemblyItems const& _items) const;

	/// @returns the contract ABI as a JSON object.
	/// This will generate the JSON object and store it in the Contract object if it is not present yet.
	Json::Value const& contractABI(Contract const&) const;

	/// @returns the Natspec User documentation as a JSON object.
	/// This will generate the JSON object and store it in the Contract object if it is not present yet.
	Json::Value const& natspecUser(Contract const&) const;

	/// @returns the Natspec Developer documentation as a JSON object.
	/// This will generate the JSON object and store it in the Contract object if it is not present yet.
	Json::Value const& natspecDev(Contract const&) const;

	/// @returns the Contract Metadata
	/// This will generate the metadata and store it in the Contract object if it is not present yet.
	std::string const& metadata(Contract const&) const;

	/// @returns the offset of the entry point of the given function into the list of assembly items
	/// or zero if it is not found or does not exist.
	size_t functionEntryPoint(
		std::string const& _contractName,
		FunctionDefinition const& _function
	) const;

	ReadCallback::Callback m_readFile;
	OptimiserSettings m_optimiserSettings;
	langutil::SVMVersion m_svmVersion;
	std::set<std::string> m_requestedContractNames;
	std::map<std::string, h160> m_libraries;
	/// list of path prefix remappings, e.g. mylibrary: octonion.institute/susy-go = /usr/local/sophon
	/// "context:prefix=target"
	std::vector<Remapping> m_remappings;
	std::map<std::string const, Source> m_sources;
	std::vector<std::string> m_unhandledSMTLib2Queries;
	std::map<h256, std::string> m_smtlib2Responses;
	std::shared_ptr<GlobalContext> m_globalContext;
	std::vector<Source const*> m_sourceOrder;
	/// This is updated during compilation.
	std::map<ASTNode const*, std::shared_ptr<DeclarationContainer>> m_scopes;
	std::map<std::string const, Contract> m_contracts;
	langutil::ErrorList m_errorList;
	langutil::ErrorReporter m_errorReporter;
	bool m_metadataLiteralSources = false;
	State m_stackState = Empty;
};

}
}
