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
#include <libyul/Exceptions.h>
#include <libyul/AsmParser.h>
#include <libyul/backends/svm/AbstractAssembly.h>

#include <libsvmasm/SemanticInformation.h>
#include <libsvmasm/Instruction.h>

#include <liblangutil/Exceptions.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace yul;

namespace
{
pair<YulString, BuiltinFunctionForSVM> createSVMFunction(
	string const& _name,
	dev::sof::Instruction _instruction
)
{
	sof::InstructionInfo info = dev::sof::instructionInfo(_instruction);
	BuiltinFunctionForSVM f;
	f.name = YulString{_name};
	f.parameters.resize(info.args);
	f.returns.resize(info.ret);
	f.movable = sof::SemanticInformation::movable(_instruction);
	f.sideEffectFree = sof::SemanticInformation::sideEffectFree(_instruction);
	f.sideEffectFreeIfNoMSize = sof::SemanticInformation::sideEffectFreeIfNoMSize(_instruction);
	f.isMSize = _instruction == dev::sof::Instruction::MSIZE;
	f.literalArguments = false;
	f.instruction = _instruction;
	f.generateCode = [_instruction](
		FunctionCall const&,
		AbstractAssembly& _assembly,
		BuiltinContext&,
		std::function<void()> _visitArguments
	) {
		_visitArguments();
		_assembly.appendInstruction(_instruction);
	};

	return {f.name, move(f)};
}

pair<YulString, BuiltinFunctionForSVM> createFunction(
	string _name,
	size_t _params,
	size_t _returns,
	bool _movable,
	bool _sideEffectFree,
	bool _sideEffectFreeIfNoMSize,
	bool _literalArguments,
	std::function<void(FunctionCall const&, AbstractAssembly&, BuiltinContext&, std::function<void()>)> _generateCode
)
{
	YulString name{std::move(_name)};
	BuiltinFunctionForSVM f;
	f.name = name;
	f.parameters.resize(_params);
	f.returns.resize(_returns);
	f.movable = _movable;
	f.literalArguments = _literalArguments;
	f.sideEffectFree = _sideEffectFree;
	f.sideEffectFreeIfNoMSize = _sideEffectFreeIfNoMSize;
	f.isMSize = false;
	f.instruction = {};
	f.generateCode = std::move(_generateCode);
	return {name, f};
}

map<YulString, BuiltinFunctionForSVM> createBuiltins(langutil::SVMVersion _svmVersion, bool _objectAccess)
{
	map<YulString, BuiltinFunctionForSVM> builtins;
	for (auto const& instr: Parser::instructions())
		if (
			!dev::sof::isDupInstruction(instr.second) &&
			!dev::sof::isSwapInstruction(instr.second) &&
			instr.second != sof::Instruction::JUMP &&
			instr.second != sof::Instruction::JUMPI &&
			_svmVersion.hasOpcode(instr.second)
		)
			builtins.emplace(createSVMFunction(instr.first, instr.second));

	if (_objectAccess)
	{
		builtins.emplace(createFunction("datasize", 1, 1, true, true, true, true, [](
			FunctionCall const& _call,
			AbstractAssembly& _assembly,
			BuiltinContext& _context,
			function<void()>
		) {
			yulAssert(_context.currentObject, "No object available.");
			yulAssert(_call.arguments.size() == 1, "");
			Expression const& arg = _call.arguments.front();
			YulString dataName = boost::get<Literal>(arg).value;
			if (_context.currentObject->name == dataName)
				_assembly.appendAssemblySize();
			else
			{
				yulAssert(
					_context.subIDs.count(dataName) != 0,
					"Could not find assembly object <" + dataName.str() + ">."
				);
				_assembly.appendDataSize(_context.subIDs.at(dataName));
			}
		}));
		builtins.emplace(createFunction("dataoffset", 1, 1, true, true, true, true, [](
			FunctionCall const& _call,
			AbstractAssembly& _assembly,
			BuiltinContext& _context,
			std::function<void()>
		) {
			yulAssert(_context.currentObject, "No object available.");
			yulAssert(_call.arguments.size() == 1, "");
			Expression const& arg = _call.arguments.front();
			YulString dataName = boost::get<Literal>(arg).value;
			if (_context.currentObject->name == dataName)
				_assembly.appendConstant(0);
			else
			{
				yulAssert(
					_context.subIDs.count(dataName) != 0,
					"Could not find assembly object <" + dataName.str() + ">."
				);
				_assembly.appendDataOffset(_context.subIDs.at(dataName));
			}
		}));
		builtins.emplace(createFunction("datacopy", 3, 0, false, false, false, false, [](
			FunctionCall const&,
			AbstractAssembly& _assembly,
			BuiltinContext&,
			std::function<void()> _visitArguments
		) {
			_visitArguments();
			_assembly.appendInstruction(dev::sof::Instruction::CODECOPY);
		}));
	}
	return builtins;
}

}

SVMDialect::SVMDialect(AsmFlavour _flavour, bool _objectAccess, langutil::SVMVersion _svmVersion):
	Dialect{_flavour},
	m_objectAccess(_objectAccess),
	m_svmVersion(_svmVersion),
	m_functions(createBuiltins(_svmVersion, _objectAccess))
{
}

BuiltinFunctionForSVM const* SVMDialect::builtin(YulString _name) const
{
	auto it = m_functions.find(_name);
	if (it != m_functions.end())
		return &it->second;
	else
		return nullptr;
}

SVMDialect const& SVMDialect::looseAssemblyForSVM(langutil::SVMVersion _version)
{
	static map<langutil::SVMVersion, unique_ptr<SVMDialect const>> dialects;
	static YulStringRepository::ResetCallback callback{[&] { dialects.clear(); }};
	if (!dialects[_version])
		dialects[_version] = make_unique<SVMDialect>(AsmFlavour::Loose, false, _version);
	return *dialects[_version];
}

SVMDialect const& SVMDialect::strictAssemblyForSVM(langutil::SVMVersion _version)
{
	static map<langutil::SVMVersion, unique_ptr<SVMDialect const>> dialects;
	static YulStringRepository::ResetCallback callback{[&] { dialects.clear(); }};
	if (!dialects[_version])
		dialects[_version] = make_unique<SVMDialect>(AsmFlavour::Strict, false, _version);
	return *dialects[_version];
}

SVMDialect const& SVMDialect::strictAssemblyForSVMObjects(langutil::SVMVersion _version)
{
	static map<langutil::SVMVersion, unique_ptr<SVMDialect const>> dialects;
	static YulStringRepository::ResetCallback callback{[&] { dialects.clear(); }};
	if (!dialects[_version])
		dialects[_version] = make_unique<SVMDialect>(AsmFlavour::Strict, true, _version);
	return *dialects[_version];
}

SVMDialect const& SVMDialect::yulForSVM(langutil::SVMVersion _version)
{
	static map<langutil::SVMVersion, unique_ptr<SVMDialect const>> dialects;
	static YulStringRepository::ResetCallback callback{[&] { dialects.clear(); }};
	if (!dialects[_version])
		dialects[_version] = make_unique<SVMDialect>(AsmFlavour::Yul, false, _version);
	return *dialects[_version];
}
