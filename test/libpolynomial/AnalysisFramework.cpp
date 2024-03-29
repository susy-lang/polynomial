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
 * Framework for testing features from the analysis phase of compiler.
 */

#include <test/libpolynomial/AnalysisFramework.h>

#include <test/Options.h>

#include <libpolynomial/interface/CompilerStack.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libpolynomial/ast/AST.h>

#include <liblangutil/Scanner.h>

#include <libdevcore/Keccak256.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::polynomial;
using namespace dev::polynomial::test;

pair<SourceUnit const*, ErrorList>
AnalysisFramework::parseAnalyseAndReturnError(
	string const& _source,
	bool _reportWarnings,
	bool _insertVersionPragma,
	bool _allowMultipleErrors
)
{
	compiler().reset();
	compiler().setSources({{"", _insertVersionPragma ? "pragma polynomial >=0.0;\n" + _source : _source}});
	compiler().setSVMVersion(dev::test::Options::get().svmVersion());
	if (!compiler().parse())
	{
		BOOST_FAIL("Parsing contract failed in analysis test suite:" + formatErrors());
	}

	compiler().analyze();

	ErrorList errors = filterErrors(compiler().errors(), _reportWarnings);
	if (errors.size() > 1 && !_allowMultipleErrors)
		BOOST_FAIL("Multiple errors found: " + formatErrors());

	return make_pair(&compiler().ast(""), std::move(errors));
}

ErrorList AnalysisFramework::filterErrors(ErrorList const& _errorList, bool _includeWarnings) const
{
	ErrorList errors;
	for (auto const& currentError: _errorList)
	{
		polAssert(currentError->comment(), "");
		if (currentError->type() == Error::Type::Warning)
		{
			if (!_includeWarnings)
				continue;
			bool ignoreWarning = false;
			for (auto const& filter: m_warningsToFilter)
				if (currentError->comment()->find(filter) == 0)
				{
					ignoreWarning = true;
					break;
				}
			if (ignoreWarning)
				continue;
		}

		errors.emplace_back(currentError);
	}

	return errors;
}

SourceUnit const* AnalysisFramework::parseAndAnalyse(string const& _source)
{
	auto sourceAndError = parseAnalyseAndReturnError(_source);
	BOOST_REQUIRE(!!sourceAndError.first);
	string message;
	if (!sourceAndError.second.empty())
		message = "Unexpected error: " + formatErrors();
	BOOST_REQUIRE_MESSAGE(sourceAndError.second.empty(), message);
	return sourceAndError.first;
}

bool AnalysisFramework::success(string const& _source)
{
	return parseAnalyseAndReturnError(_source).second.empty();
}

ErrorList AnalysisFramework::expectError(std::string const& _source, bool _warning, bool _allowMultiple)
{
	auto sourceAndErrors = parseAnalyseAndReturnError(_source, _warning, true, _allowMultiple);
	BOOST_REQUIRE(!sourceAndErrors.second.empty());
	BOOST_REQUIRE_MESSAGE(!!sourceAndErrors.first, "Expected error, but no error happened.");
	return sourceAndErrors.second;
}

string AnalysisFramework::formatErrors() const
{
	string message;
	for (auto const& error: compiler().errors())
		message += formatError(*error);
	return message;
}

string AnalysisFramework::formatError(Error const& _error) const
{
	return SourceReferenceFormatter::formatErrorInformation(_error);
}

ContractDefinition const* AnalysisFramework::retrieveContractByName(SourceUnit const& _source, string const& _name)
{
	ContractDefinition* contract = nullptr;

	for (shared_ptr<ASTNode> const& node: _source.nodes())
		if ((contract = dynamic_cast<ContractDefinition*>(node.get())) && contract->name() == _name)
			return contract;

	return nullptr;
}

FunctionTypePointer AnalysisFramework::retrieveFunctionBySignature(
	ContractDefinition const& _contract,
	std::string const& _signature
)
{
	FixedHash<4> hash(dev::keccak256(_signature));
	return _contract.interfaceFunctions()[hash];
}
