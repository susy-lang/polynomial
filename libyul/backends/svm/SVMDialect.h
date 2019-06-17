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
 * Yul dialects for SVM.
 */

#pragma once

#include <libyul/Dialect.h>

#include <libyul/backends/svm/AbstractAssembly.h>
#include <liblangutil/SVMVersion.h>

#include <map>

namespace yul
{

class YulString;
using Type = YulString;
struct FunctionCall;
struct Object;

/**
 * Context used during code generation.
 */
struct BuiltinContext
{
	Object const* currentObject = nullptr;
	/// Mapping from named objects to abstract assembly sub IDs.
	std::map<YulString, AbstractAssembly::SubID> subIDs;
};

struct BuiltinFunctionForSVM: BuiltinFunction
{
	boost::optional<dev::sof::Instruction> instruction;
	/// Function to generate code for the given function call and append it to the abstract
	/// assembly. The fourth parameter is called to visit (and generate code for) the arguments
	/// from right to left.
	std::function<void(FunctionCall const&, AbstractAssembly&, BuiltinContext&, std::function<void()>)> generateCode;
};


/**
 * Yul dialect for SVM as a backend.
 * The main difference is that the builtin functions take an AbstractAssembly for the
 * code generation.
 */
struct SVMDialect: public Dialect
{
	/// Constructor, should only be used internally. Use the factory functions below.
	SVMDialect(AsmFlavour _flavour, bool _objectAccess, langutil::SVMVersion _svmVersion);

	/// @returns the builtin function of the given name or a nullptr if it is not a builtin function.
	BuiltinFunctionForSVM const* builtin(YulString _name) const override;

	static SVMDialect const& looseAssemblyForSVM(langutil::SVMVersion _version);
	static SVMDialect const& strictAssemblyForSVM(langutil::SVMVersion _version);
	static SVMDialect const& strictAssemblyForSVMObjects(langutil::SVMVersion _version);
	static SVMDialect const& yulForSVM(langutil::SVMVersion _version);

	langutil::SVMVersion svmVersion() const { return m_svmVersion; }

	bool providesObjectAccess() const { return m_objectAccess; }

protected:
	bool const m_objectAccess;
	langutil::SVMVersion const m_svmVersion;
	std::map<YulString, BuiltinFunctionForSVM> m_functions;
};

}
