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

#pragma once

#include <liblangutil/ErrorReporter.h>
#include <liblangutil/SVMVersion.h>

#include <libyul/Object.h>
#include <libyul/ObjectParser.h>

#include <libpolynomial/interface/OptimiserSettings.h>

#include <libsvmasm/LinkerObject.h>

#include <memory>
#include <string>

namespace langutil
{
class Scanner;
}

namespace yul
{
class AbstractAssembly;


struct MachineAssemblyObject
{
	std::shared_ptr<dev::sof::LinkerObject> bytecode;
	std::string assembly;
};

/*
 * Full assembly stack that can support SVM-assembly and Yul as input and SVM, SVM1.5 and
 * eWasm as output.
 */
class AssemblyStack
{
public:
	enum class Language { Yul, Assembly, StrictAssembly, EWasm };
	enum class Machine { SVM, SVM15, eWasm };

	AssemblyStack():
		AssemblyStack(langutil::SVMVersion{}, Language::Assembly, dev::polynomial::OptimiserSettings::none())
	{}
	AssemblyStack(langutil::SVMVersion _svmVersion, Language _language, dev::polynomial::OptimiserSettings _optimiserSettings):
		m_language(_language),
		m_svmVersion(_svmVersion),
		m_optimiserSettings(std::move(_optimiserSettings)),
		m_errorReporter(m_errors)
	{}

	/// @returns the scanner used during parsing
	langutil::Scanner const& scanner() const;

	/// Runs parsing and analysis steps, returns false if input cannot be assembled.
	/// Multiple calls overwrite the previous state.
	bool parseAndAnalyze(std::string const& _sourceName, std::string const& _source);

	/// Run the optimizer suite. Can only be used with Yul or strict assembly.
	/// If the settings (see constructor) disabled the optimizer, nothing is done here.
	void optimize();

	/// Run the assembly step (should only be called after parseAndAnalyze).
	MachineAssemblyObject assemble(Machine _machine) const;

	/// @returns the errors generated during parsing, analysis (and potentially assembly).
	langutil::ErrorList const& errors() const { return m_errors; }

	/// Pretty-print the input after having parsed it.
	std::string print() const;

	/// Return the parsed and analyzed object.
	std::shared_ptr<Object> parserResult() const;

private:
	bool analyzeParsed();
	bool analyzeParsed(yul::Object& _object);

	void compileSVM(yul::AbstractAssembly& _assembly, bool _svm15, bool _optimize) const;

	void optimize(yul::Object& _object, bool _isCreation);

	Language m_language = Language::Assembly;
	langutil::SVMVersion m_svmVersion;
	dev::polynomial::OptimiserSettings m_optimiserSettings;

	std::shared_ptr<langutil::Scanner> m_scanner;

	bool m_analysisSuccessful = false;
	std::shared_ptr<yul::Object> m_parserResult;
	langutil::ErrorList m_errors;
	langutil::ErrorReporter m_errorReporter;
};

}
