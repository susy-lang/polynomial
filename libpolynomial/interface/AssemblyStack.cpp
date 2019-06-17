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

#include <liblangutil/Scanner.h>
#include <libyul/AsmPrinter.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmCodeGen.h>
#include <libyul/backends/svm/SVMCodeTransform.h>
#include <libyul/backends/svm/SVMAssembly.h>
#include <libyul/ObjectParser.h>

#include <libsvmasm/Assembly.h>

#include <libyul/optimiser/Suite.h>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::polynomial;

namespace
{
yul::AsmFlavour languageToAsmFlavour(AssemblyStack::Language _language)
{
	switch (_language)
	{
	case AssemblyStack::Language::Assembly:
		return yul::AsmFlavour::Loose;
	case AssemblyStack::Language::StrictAssembly:
		return yul::AsmFlavour::Strict;
	case AssemblyStack::Language::Yul:
		return yul::AsmFlavour::Yul;
	}
	polAssert(false, "");
	return yul::AsmFlavour::Yul;
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
	m_scanner = make_shared<Scanner>(CharStream(_source, _sourceName));
	m_parserResult = yul::ObjectParser(m_errorReporter, languageToAsmFlavour(m_language)).parse(m_scanner, false);
	if (!m_errorReporter.errors().empty())
		return false;
	polAssert(m_parserResult, "");
	polAssert(m_parserResult->code, "");

	return analyzeParsed();
}

void AssemblyStack::optimize()
{
	polAssert(m_language != Language::Assembly, "Optimization requested for loose assembly.");
	yul::OptimiserSuite::run(*m_parserResult->code, *m_parserResult->analysisInfo);
	polAssert(analyzeParsed(), "Invalid source code after optimization.");
}

bool AssemblyStack::analyzeParsed()
{
	polAssert(m_parserResult, "");
	polAssert(m_parserResult->code, "");
	m_parserResult->analysisInfo = make_shared<yul::AsmAnalysisInfo>();
	yul::AsmAnalyzer analyzer(*m_parserResult->analysisInfo, m_errorReporter, m_svmVersion, boost::none, languageToAsmFlavour(m_language));
	m_analysisSuccessful = analyzer.analyze(*m_parserResult->code);
	return m_analysisSuccessful;
}

MachineAssemblyObject AssemblyStack::assemble(Machine _machine) const
{
	polAssert(m_analysisSuccessful, "");
	polAssert(m_parserResult, "");
	polAssert(m_parserResult->code, "");
	polAssert(m_parserResult->analysisInfo, "");

	switch (_machine)
	{
	case Machine::SVM:
	{
		MachineAssemblyObject object;
		sof::Assembly assembly;
		yul::CodeGenerator::assemble(*m_parserResult->code, *m_parserResult->analysisInfo, assembly);
		object.bytecode = make_shared<sof::LinkerObject>(assembly.assemble());
		object.assembly = assembly.assemblyString();
		return object;
	}
	case Machine::SVM15:
	{
		MachineAssemblyObject object;
		yul::SVMAssembly assembly(true);
		yul::CodeTransform(assembly, *m_parserResult->analysisInfo, m_language == Language::Yul, true)(*m_parserResult->code);
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
	polAssert(m_parserResult->code, "");
	return m_parserResult->toString(m_language == Language::Yul) + "\n";
}
