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
 * Framework for executing Polynomial contracts and testing them against C++ implementation.
 */

#include <cstdlib>
#include <boost/test/framework.hpp>
#include <test/libpolynomial/PolynomialExecutionFramework.h>

using namespace dev;
using namespace dev::test;
using namespace dev::polynomial;
using namespace dev::polynomial::test;
using namespace std;

PolynomialExecutionFramework::PolynomialExecutionFramework():
	ExecutionFramework()
{
}

PolynomialExecutionFramework::PolynomialExecutionFramework(std::string const& _ipcPath, langutil::SVMVersion _svmVersion):
	ExecutionFramework(_ipcPath, _svmVersion)
{
}

bytes PolynomialExecutionFramework::compileContract(
	string const& _sourceCode,
	string const& _contractName,
	map<string, Address> const& _libraryAddresses
)
{
	// Silence compiler version warning
	std::string sourceCode = "pragma polynomial >=0.0;\n";
	if (
		dev::test::Options::get().useABIEncoderV2 &&
		_sourceCode.find("pragma experimental ABIEncoderV2;") == std::string::npos
	)
		sourceCode += "pragma experimental ABIEncoderV2;\n";
	sourceCode += _sourceCode;
	m_compiler.reset();
	m_compiler.setSources({{"", sourceCode}});
	m_compiler.setLibraries(_libraryAddresses);
	m_compiler.setSVMVersion(m_svmVersion);
	m_compiler.setOptimiserSettings(m_optimiserSettings);
	m_compiler.enableIRGeneration(m_compileViaYul);
	if (!m_compiler.compile())
	{
		langutil::SourceReferenceFormatter formatter(std::cerr);

		for (auto const& error: m_compiler.errors())
			formatter.printErrorInformation(*error);
		BOOST_ERROR("Compiling contract failed");
	}
	sof::LinkerObject obj;
	if (m_compileViaYul)
	{
		yul::AssemblyStack asmStack(
					m_svmVersion,
					yul::AssemblyStack::Language::StrictAssembly,
					// Ignore optimiser settings here because we need Yul optimisation to
					// get code that does not exhaust the stack.
					OptimiserSettings::full()
					);
		if (!asmStack.parseAndAnalyze("", m_compiler.yulIROptimized(
			_contractName.empty() ? m_compiler.lastContractName() : _contractName
		)))
		{
			langutil::SourceReferenceFormatter formatter(std::cerr);

			for (auto const& error: m_compiler.errors())
				formatter.printErrorInformation(*error);
			BOOST_ERROR("Assembly contract failed. IR: " + m_compiler.yulIROptimized({}));
		}
		asmStack.optimize();
		obj = std::move(*asmStack.assemble(yul::AssemblyStack::Machine::SVM).bytecode);
	}
	else
		obj = m_compiler.object(_contractName.empty() ? m_compiler.lastContractName() : _contractName);
	BOOST_REQUIRE(obj.linkReferences.empty());
	return obj.bytecode;
}
