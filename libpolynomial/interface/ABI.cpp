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
 * Utilities to handle the Contract ABI (https://octonion.institute/susy-go/wiki/Sophon-Contract-ABI)
 */

#include <libpolynomial/interface/ABI.h>
#include <libpolynomial/ast/AST.h>

using namespace std;
using namespace dev;
using namespace dev::polynomial;

Json::Value ABI::generate(ContractDefinition const& _contractDef)
{
	Json::Value abi(Json::arrayValue);

	for (auto it: _contractDef.interfaceFunctions())
	{
		auto externalFunctionType = it.second->interfaceFunctionType();
		Json::Value method;
		method["type"] = "function";
		method["name"] = it.second->declaration().name();
		// TODO: deprecate constant in a future release
		method["constant"] = it.second->stateMutability() == StateMutability::Pure || it.second->stateMutability() == StateMutability::View;
		method["payable"] = it.second->isPayable();
		method["stateMutability"] = stateMutabilityToString(it.second->stateMutability());
		method["inputs"] = formatTypeList(
			externalFunctionType->parameterNames(),
			externalFunctionType->parameterTypes(),
			_contractDef.isLibrary()
		);
		method["outputs"] = formatTypeList(
			externalFunctionType->returnParameterNames(),
			externalFunctionType->returnParameterTypes(),
			_contractDef.isLibrary()
		);
		abi.append(method);
	}
	if (_contractDef.constructor())
	{
		Json::Value method;
		method["type"] = "constructor";
		auto externalFunction = FunctionType(*_contractDef.constructor(), false).interfaceFunctionType();
		polAssert(!!externalFunction, "");
		method["payable"] = externalFunction->isPayable();
		method["stateMutability"] = stateMutabilityToString(externalFunction->stateMutability());
		method["inputs"] = formatTypeList(
			externalFunction->parameterNames(),
			externalFunction->parameterTypes(),
			_contractDef.isLibrary()
		);
		abi.append(method);
	}
	if (_contractDef.fallbackFunction())
	{
		auto externalFunctionType = FunctionType(*_contractDef.fallbackFunction(), false).interfaceFunctionType();
		polAssert(!!externalFunctionType, "");
		Json::Value method;
		method["type"] = "fallback";
		method["payable"] = externalFunctionType->isPayable();
		method["stateMutability"] = stateMutabilityToString(externalFunctionType->stateMutability());
		abi.append(method);
	}
	for (auto const& it: _contractDef.interfaceEvents())
	{
		Json::Value event;
		event["type"] = "event";
		event["name"] = it->name();
		event["anonymous"] = it->isAnonymous();
		Json::Value params(Json::arrayValue);
		for (auto const& p: it->parameters())
		{
			polAssert(!!p->annotation().type->interfaceType(false), "");
			Json::Value input;
			input["name"] = p->name();
			input["type"] = p->annotation().type->interfaceType(false)->canonicalName(false);
			input["indexed"] = p->isIndexed();
			params.append(input);
		}
		event["inputs"] = params;
		abi.append(event);
	}

	return abi;
}

Json::Value ABI::formatTypeList(
	vector<string> const& _names,
	vector<TypePointer> const& _types,
	bool _forLibrary
)
{
	Json::Value params(Json::arrayValue);
	polAssert(_names.size() == _types.size(), "Names and types vector size does not match");
	for (unsigned i = 0; i < _names.size(); ++i)
	{
		polAssert(_types[i], "");
		Json::Value param;
		param["name"] = _names[i];
		param["type"] = _types[i]->canonicalName(_forLibrary);
		params.append(param);
	}
	return params;
}
