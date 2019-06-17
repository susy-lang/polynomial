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

struct BuiltinFunctionForSVM: BuiltinFunction
{
	/// Function to generate code for the given function call and append it to the abstract
	/// assembly. The third parameter is called to visit (and generate code for) the arguments
	/// from right to left.
	std::function<void(FunctionCall const&, AbstractAssembly&, std::function<void()>)> generateCode;
};

/**
 * Yul dialect for SVM as a backend.
 * The main difference is that the builtin functions take an AbstractAssembly for the
 * code generation.
 */
struct SVMDialect: public Dialect
{
	SVMDialect(AsmFlavour _flavour, bool _objectAccess, langutil::SVMVersion _svmVersion);

	/// @returns the builtin function of the given name or a nullptr if it is not a builtin function.
	BuiltinFunctionForSVM const* builtin(YulString _name) const override;

	static std::shared_ptr<SVMDialect> looseAssemblyForSVM(langutil::SVMVersion _version);
	static std::shared_ptr<SVMDialect> strictAssemblyForSVM(langutil::SVMVersion _version);
	static std::shared_ptr<SVMDialect> strictAssemblyForSVMObjects(langutil::SVMVersion _version);
	static std::shared_ptr<SVMDialect> yulForSVM(langutil::SVMVersion _version);

	langutil::SVMVersion svmVersion() const { return m_svmVersion; }

	bool providesObjectAccess() const { return m_objectAccess; }

	/// Sets the mapping of current sub assembly IDs. Used during code generation.
	void setSubIDs(std::map<YulString, AbstractAssembly::SubID> _subIDs);
	/// Sets the current object. Used during code generation.
	void setCurrentObject(Object const* _object);

protected:
	void addFunction(
		std::string _name,
		size_t _params,
		size_t _returns,
		bool _movable,
		bool _literalArguments,
		std::function<void(FunctionCall const&, AbstractAssembly&, std::function<void()>)> _generateCode
	);

	bool m_objectAccess;
	langutil::SVMVersion m_svmVersion;
	Object const* m_currentObject = nullptr;
	/// Mapping from named objects to abstract assembly sub IDs.
	std::map<YulString, AbstractAssembly::SubID> m_subIDs;
	std::map<YulString, BuiltinFunctionForSVM> m_functions;
};

}
