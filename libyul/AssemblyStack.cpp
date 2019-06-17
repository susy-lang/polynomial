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


#include <libyul/AssemblyStack.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmPrinter.h>
#include <libyul/backends/svm/AsmCodeGen.h>
#include <libyul/backends/svm/SVMAssembly.h>
#include <libyul/backends/svm/SVMCodeTransform.h>
#include <libyul/backends/svm/SVMDialect.h>
#include <libyul/backends/svm/SVMObjectCompiler.h>
#include <libyul/backends/wasm/WasmDialect.h>
#include <libyul/backends/wasm/EWasmObjectCompiler.h>
#include <libyul/optimiser/Metrics.h>
#include <libyul/ObjectParser.h>
#include <libyul/optimiser/Suite.h>

#include <libpolynomial/interface/OptimiserSettings.h>

#include <libsvmasm/Assembly.h>
#include <liblangutil/Scanner.h>

using namespace std;
using namespace langutil;
using namespace yul;

namespace
{
Dialect const& languageToDialect(AssemblyStack::Language _language, SVMVersion _version)
{
	switch (_language)
	{
	case AssemblyStack::Language::Assembly:
		return SVMDialect::looseAssemblyForSVM(_version);
	case AssemblyStack::Language::StrictAssembly:
		return SVMDialect::strictAssemblyForSVMObjects(_version);
	case AssemblyStack::Language::Yul:
		return Dialect::yul();
	case AssemblyStack::Language::EWasm:
		return WasmDialect::instance();
	}
	polAssert(false, "");
	return Dialect::yul();
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
	m_parserResult = ObjectParser(m_errorReporter, languageToDialect(m_language, m_svmVersion)).parse(m_scanner, false);
	if (!m_errorReporter.errors().empty())
		return false;
	polAssert(m_parserResult, "");
	polAssert(m_parserResult->code, "");

	return analyzeParsed();
}

void AssemblyStack::optimize()
{
	if (!m_optimiserSettings.runYulOptimiser)
		return;

	if (m_language != Language::StrictAssembly)
		polUnimplemented("Optimizer for both loose assembly and Yul is not yet implemented");
	polAssert(m_analysisSuccessful, "Analysis was not successful.");

	m_analysisSuccessful = false;
	polAssert(m_parserResult, "");
	optimize(*m_parserResult, true);
	polAssert(analyzeParsed(), "Invalid source code after optimization.");
}

bool AssemblyStack::analyzeParsed()
{
	polAssert(m_parserResult, "");
	m_analysisSuccessful = analyzeParsed(*m_parserResult);
	return m_analysisSuccessful;
}

bool AssemblyStack::analyzeParsed(Object& _object)
{
	polAssert(_object.code, "");
	_object.analysisInfo = make_shared<AsmAnalysisInfo>();
	AsmAnalyzer analyzer(*_object.analysisInfo, m_errorReporter, boost::none, languageToDialect(m_language, m_svmVersion));
	bool success = analyzer.analyze(*_object.code);
	for (auto& subNode: _object.subObjects)
		if (auto subObject = dynamic_cast<Object*>(subNode.get()))
			if (!analyzeParsed(*subObject))
				success = false;
	return success;
}

void AssemblyStack::compileSVM(AbstractAssembly& _assembly, bool _svm15, bool _optimize) const
{
	SVMDialect const* dialect = nullptr;

	if (m_language == Language::Assembly)
		dialect = &SVMDialect::looseAssemblyForSVM(m_svmVersion);
	else if (m_language == AssemblyStack::Language::StrictAssembly)
		dialect = &SVMDialect::strictAssemblyForSVMObjects(m_svmVersion);
	else if (m_language == AssemblyStack::Language::Yul)
		dialect = &SVMDialect::yulForSVM(m_svmVersion);
	else
		polAssert(false, "Invalid language.");

	SVMObjectCompiler::compile(*m_parserResult, _assembly, *dialect, _svm15, _optimize);
}

void AssemblyStack::optimize(Object& _object, bool _isCreation)
{
	polAssert(_object.code, "");
	polAssert(_object.analysisInfo, "");
	for (auto& subNode: _object.subObjects)
		if (auto subObject = dynamic_cast<Object*>(subNode.get()))
			optimize(*subObject, false);
	SVMDialect const& dialect = dynamic_cast<SVMDialect const&>(languageToDialect(m_language, m_svmVersion));
	GasMeter meter(dialect, _isCreation, m_optimiserSettings.expectedExecutionsPerDeployment);
	OptimiserSuite::run(
		dialect,
		meter,
		*_object.code,
		*_object.analysisInfo,
		m_optimiserSettings.optimizeStackAllocation
	);
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
		dev::sof::Assembly assembly;
		SofAssemblyAdapter adapter(assembly);
		compileSVM(adapter, false, m_optimiserSettings.optimizeStackAllocation);
		object.bytecode = make_shared<dev::sof::LinkerObject>(assembly.assemble());
		object.assembly = assembly.assemblyString();
		return object;
	}
	case Machine::SVM15:
	{
		MachineAssemblyObject object;
		SVMAssembly assembly(true);
		compileSVM(assembly, true, m_optimiserSettings.optimizeStackAllocation);
		object.bytecode = make_shared<dev::sof::LinkerObject>(assembly.finalize());
		/// TODO: fill out text representation
		return object;
	}
	case Machine::eWasm:
	{
		polAssert(m_language == Language::EWasm, "");
		Dialect const& dialect = languageToDialect(m_language, SVMVersion{});

		MachineAssemblyObject object;
		object.assembly = EWasmObjectCompiler::compile(*m_parserResult, dialect);
		return object;
	}
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

shared_ptr<Object> AssemblyStack::parserResult() const
{
	polAssert(m_analysisSuccessful, "Analysis was not successful.");
	polAssert(m_parserResult, "");
	polAssert(m_parserResult->code, "");
	return m_parserResult;
}
