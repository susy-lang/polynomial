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

#include <libyul/backends/svm/SVMDialect.h>

#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmData.h>
#include <libyul/Object.h>
#include <libyul/backends/svm/AbstractAssembly.h>

#include <liblangutil/Exceptions.h>

#include <libyul/Exceptions.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace yul;

SVMDialect::SVMDialect(AsmFlavour _flavour, bool _objectAccess, langutil::SVMVersion _svmVersion):
	Dialect{_flavour}, m_objectAccess(_objectAccess), m_svmVersion(_svmVersion)
{
	// The SVM instructions will be moved to builtins at some point.
	if (!m_objectAccess)
		return;

	addFunction("datasize", 1, 1, true, true, [this](
		FunctionCall const& _call,
		AbstractAssembly& _assembly,
		std::function<void()>
	) {
		yulAssert(m_currentObject, "No object available.");
		yulAssert(_call.arguments.size() == 1, "");
		Expression const& arg = _call.arguments.front();
		YulString dataName = boost::get<Literal>(arg).value;
		if (m_currentObject->name == dataName)
			_assembly.appendAssemblySize();
		else
		{
			yulAssert(m_subIDs.count(dataName) != 0, "Could not find assembly object <" + dataName.str() + ">.");
			_assembly.appendDataSize(m_subIDs.at(dataName));
		}
	});
	addFunction("dataoffset", 1, 1, true, true, [this](
		FunctionCall const& _call,
		AbstractAssembly& _assembly,
		std::function<void()>
	) {
		yulAssert(m_currentObject, "No object available.");
		yulAssert(_call.arguments.size() == 1, "");
		Expression const& arg = _call.arguments.front();
		YulString dataName = boost::get<Literal>(arg).value;
		if (m_currentObject->name == dataName)
			_assembly.appendConstant(0);
		else
		{
			yulAssert(m_subIDs.count(dataName) != 0, "Could not find assembly object <" + dataName.str() + ">.");
			_assembly.appendDataOffset(m_subIDs.at(dataName));
		}
	});
	addFunction("datacopy", 3, 0, false, false, [](
		FunctionCall const&,
		AbstractAssembly& _assembly,
		std::function<void()> _visitArguments
	) {
		_visitArguments();
		_assembly.appendInstruction(dev::sof::Instruction::CODECOPY);
	});
}

BuiltinFunctionForSVM const* SVMDialect::builtin(YulString _name) const
{
	auto it = m_functions.find(_name);
	if (it != m_functions.end())
		return &it->second;
	else
		return nullptr;
}

shared_ptr<SVMDialect> SVMDialect::looseAssemblyForSVM(langutil::SVMVersion _version)
{
	return make_shared<SVMDialect>(AsmFlavour::Loose, false, _version);
}

shared_ptr<SVMDialect> SVMDialect::strictAssemblyForSVM(langutil::SVMVersion _version)
{
	return make_shared<SVMDialect>(AsmFlavour::Strict, false, _version);
}

shared_ptr<SVMDialect> SVMDialect::strictAssemblyForSVMObjects(langutil::SVMVersion _version)
{
	return make_shared<SVMDialect>(AsmFlavour::Strict, true, _version);
}

shared_ptr<yul::SVMDialect> SVMDialect::yulForSVM(langutil::SVMVersion _version)
{
	return make_shared<SVMDialect>(AsmFlavour::Yul, false, _version);
}

void SVMDialect::setSubIDs(map<YulString, AbstractAssembly::SubID> _subIDs)
{
	yulAssert(m_objectAccess, "Sub IDs set with dialect that does not support object access.");
	m_subIDs = std::move(_subIDs);
}

void SVMDialect::setCurrentObject(Object const* _object)
{
	yulAssert(m_objectAccess, "Current object set with dialect that does not support object access.");
	m_currentObject = _object;
}

void SVMDialect::addFunction(
	string _name,
	size_t _params,
	size_t _returns,
	bool _movable,
	bool _literalArguments,
	std::function<void(FunctionCall const&, AbstractAssembly&, std::function<void()>)> _generateCode
)
{
	YulString name{std::move(_name)};
	BuiltinFunctionForSVM& f = m_functions[name];
	f.name = name;
	f.parameters.resize(_params);
	f.returns.resize(_returns);
	f.movable = _movable;
	f.literalArguments = _literalArguments;
	f.generateCode = std::move(_generateCode);
}
