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

#include <libpolynomial/codegen/AsmCodeGen.h>
#include <libsvmasm/Assembly.h>
#include <liblangutil/Scanner.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmPrinter.h>
#include <libyul/backends/svm/SVMAssembly.h>
#include <libyul/backends/svm/SVMCodeTransform.h>
#include <libyul/backends/svm/SVMDialect.h>
#include <libyul/backends/svm/SVMObjectCompiler.h>
#include <libyul/ObjectParser.h>
#include <libyul/optimiser/Suite.h>

using namespace std;
using namespace dev;
using namespace langutil;
using namespace dev::polynomial;

namespace
{
shared_ptr<yul::Dialect> languageToDialect(AssemblyStack::Language _language)
{
	switch (_language)
	{
	case AssemblyStack::Language::Assembly:
		return yul::SVMDialect::looseAssemblyForSVM();
	case AssemblyStack::Language::StrictAssembly:
		return yul::SVMDialect::strictAssemblyForSVMObjects();
	case AssemblyStack::Language::Yul:
		return yul::Dialect::yul();
	}
	polAssert(false, "");
	return yul::Dialect::yul();
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
	m_parserResult = yul::ObjectParser(m_errorReporter, languageToDialect(m_language)).parse(m_scanner, false);
	if (!m_errorReporter.errors().empty())
		return false;
	polAssert(m_parserResult, "");
	polAssert(m_parserResult->code, "");

	return analyzeParsed();
}

void AssemblyStack::optimize()
{
	if (m_language != Language::StrictAssembly)
		polUnimplemented("Optimizer for both loose assembly and Yul is not yet implemented");
	polAssert(m_analysisSuccessful, "Analysis was not successful.");
	m_analysisSuccessful = false;
	optimize(*m_parserResult);
	polAssert(analyzeParsed(), "Invalid source code after optimization.");
}

bool AssemblyStack::analyzeParsed()
{
	polAssert(m_parserResult, "");
	m_analysisSuccessful = analyzeParsed(*m_parserResult);
	return m_analysisSuccessful;
}

bool AssemblyStack::analyzeParsed(yul::Object& _object)
{
	polAssert(_object.code, "");
	_object.analysisInfo = make_shared<yul::AsmAnalysisInfo>();
	yul::AsmAnalyzer analyzer(*_object.analysisInfo, m_errorReporter, m_svmVersion, boost::none, languageToDialect(m_language));
	bool success = analyzer.analyze(*_object.code);
	for (auto& subNode: _object.subObjects)
		if (auto subObject = dynamic_cast<yul::Object*>(subNode.get()))
			if (!analyzeParsed(*subObject))
				success = false;
	return success;
}

void AssemblyStack::compileSVM(yul::AbstractAssembly& _assembly, bool _svm15, bool _optimize) const
{
	shared_ptr<yul::SVMDialect> dialect;

	if (m_language == Language::Assembly)
		dialect = yul::SVMDialect::looseAssemblyForSVM();
	else if (m_language == AssemblyStack::Language::StrictAssembly)
		dialect = yul::SVMDialect::strictAssemblyForSVMObjects();
	else if (m_language == AssemblyStack::Language::Yul)
		dialect = yul::SVMDialect::yulForSVM();
	else
		polAssert(false, "Invalid language.");

	yul::SVMObjectCompiler::compile(*m_parserResult, _assembly, *dialect, _svm15, _optimize);
}

void AssemblyStack::optimize(yul::Object& _object)
{
	polAssert(_object.code, "");
	polAssert(_object.analysisInfo, "");
	for (auto& subNode: _object.subObjects)
		if (auto subObject = dynamic_cast<yul::Object*>(subNode.get()))
			optimize(*subObject);
	yul::OptimiserSuite::run(languageToDialect(m_language), *_object.code, *_object.analysisInfo);
}

MachineAssemblyObject AssemblyStack::assemble(Machine _machine, bool _optimize) const
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
		SofAssemblyAdapter adapter(assembly);
		compileSVM(adapter, false, _optimize);
		object.bytecode = make_shared<sof::LinkerObject>(assembly.assemble());
		object.assembly = assembly.assemblyString();
		return object;
	}
	case Machine::SVM15:
	{
		MachineAssemblyObject object;
		yul::SVMAssembly assembly(true);
		compileSVM(assembly, true, _optimize);
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
