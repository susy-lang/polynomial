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
 * Full assembly stack that can support SVM-assembly and Yul as input and SVM, SVM1.5 and
 * eWasm as output.
 */


#include <libpolynomial/interface/AssemblyStack.h>

#include <libpolynomial/parsing/Scanner.h>
#include <libpolynomial/inlineasm/AsmPrinter.h>
#include <libpolynomial/inlineasm/AsmParser.h>
#include <libpolynomial/inlineasm/AsmAnalysis.h>
#include <libpolynomial/inlineasm/AsmAnalysisInfo.h>
#include <libpolynomial/inlineasm/AsmCodeGen.h>

#include <libsvmasm/Assembly.h>

#include <libyul/backends/svm/SVMCodeTransform.h>
#include <libyul/backends/svm/SVMAssembly.h>

using namespace std;
using namespace dev;
using namespace dev::polynomial;

namespace
{
assembly::AsmFlavour languageToAsmFlavour(AssemblyStack::Language _language)
{
	switch (_language)
	{
	case AssemblyStack::Language::Assembly:
		return assembly::AsmFlavour::Loose;
	case AssemblyStack::Language::StrictAssembly:
		return assembly::AsmFlavour::Strict;
	case AssemblyStack::Language::Yul:
		return assembly::AsmFlavour::Yul;
	}
	polAssert(false, "");
	return assembly::AsmFlavour::Yul;
}

}


Scanner const& AssemblyStack::scanner() const
{
	polAssert(m_scanner, "");
	return *m_scanner;
}

bool AssemblyStack::parseAndAnalyze(std::string const& _sourceName, std::string const& _source)
{
	m_errors.clear();
	m_analysisSuccessful = false;
	m_scanner = make_shared<Scanner>(CharStream(_source), _sourceName);
	m_parserResult = assembly::Parser(m_errorReporter, languageToAsmFlavour(m_language)).parse(m_scanner, false);
	if (!m_errorReporter.errors().empty())
		return false;
	polAssert(m_parserResult, "");

	return analyzeParsed();
}

bool AssemblyStack::analyze(assembly::Block const& _block, Scanner const* _scanner)
{
	m_errors.clear();
	m_analysisSuccessful = false;
	if (_scanner)
		m_scanner = make_shared<Scanner>(*_scanner);
	m_parserResult = make_shared<assembly::Block>(_block);

	return analyzeParsed();
}

bool AssemblyStack::analyzeParsed()
{
	m_analysisInfo = make_shared<assembly::AsmAnalysisInfo>();
	assembly::AsmAnalyzer analyzer(*m_analysisInfo, m_errorReporter, m_svmVersion, boost::none, languageToAsmFlavour(m_language));
	m_analysisSuccessful = analyzer.analyze(*m_parserResult);
	return m_analysisSuccessful;
}

MachineAssemblyObject AssemblyStack::assemble(Machine _machine) const
{
	polAssert(m_analysisSuccessful, "");
	polAssert(m_parserResult, "");
	polAssert(m_analysisInfo, "");

	switch (_machine)
	{
	case Machine::SVM:
	{
		MachineAssemblyObject object;
		sof::Assembly assembly;
		assembly::CodeGenerator::assemble(*m_parserResult, *m_analysisInfo, assembly);
		object.bytecode = make_shared<sof::LinkerObject>(assembly.assemble());
		object.assembly = assembly.assemblyString();
		return object;
	}
	case Machine::SVM15:
	{
		MachineAssemblyObject object;
		yul::SVMAssembly assembly(true);
		yul::CodeTransform(assembly, *m_analysisInfo, m_language == Language::Yul, true)(*m_parserResult);
		object.bytecode = make_shared<sof::LinkerObject>(assembly.finalize());
		/// TODO: fill out text representation
		return object;
	}
	case Machine::eWasm:
		polUnimplemented("eWasm backend is not yet implemented.");
	}
	// unreachable
	return MachineAssemblyObject();
}

string AssemblyStack::print() const
{
	polAssert(m_parserResult, "");
	return assembly::AsmPrinter(m_language == Language::Yul)(*m_parserResult);
}
