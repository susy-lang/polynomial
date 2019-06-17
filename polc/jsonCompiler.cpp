/*
	This file is part of cpp-sophon.

	cpp-sophon is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-sophon is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MSRCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-sophon.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@sofdev.com>
 * @date 2014
 * JSON interface for the polynomial compiler to be used from Javascript.
 */

#include <string>
#include <iostream>
#include <json/json.h>
#include <libdevcore/Common.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/CommonIO.h>
#include <libsvmcore/Instruction.h>
#include <libsvmcore/Params.h>
#include <libpolynomial/Scanner.h>
#include <libpolynomial/Parser.h>
#include <libpolynomial/ASTPrinter.h>
#include <libpolynomial/NameAndTypeResolver.h>
#include <libpolynomial/Exceptions.h>
#include <libpolynomial/CompilerStack.h>
#include <libpolynomial/SourceReferenceFormatter.h>
#include <libpolynomial/ASTJsonConverter.h>
#include <libpolynomial/Version.h>

using namespace std;
using namespace dev;
using namespace polynomial;

string formatError(Exception const& _exception, string const& _name, CompilerStack const& _compiler)
{
	ostringstream errorOutput;
	SourceReferenceFormatter::printExceptionInformation(errorOutput, _exception, _name, _compiler);
	return errorOutput.str();
}

Json::Value functionHashes(ContractDefinition const& _contract)
{
	Json::Value functionHashes(Json::objectValue);
	for (auto const& it: _contract.interfaceFunctions())
		functionHashes[it.second->externalSignature()] = toHex(it.first.ref());
	return functionHashes;
}

Json::Value gasToJson(GasEstimator::GasConsumption const& _gas)
{
	if (_gas.isInfinite || _gas.value > std::numeric_limits<Json::LargestUInt>::max())
		return Json::Value(Json::nullValue);
	else
		return Json::Value(Json::LargestUInt(_gas.value));
}

Json::Value estimateGas(CompilerStack const& _compiler, string const& _contract)
{
	Json::Value gasEstimates(Json::objectValue);
	using Gas = GasEstimator::GasConsumption;
	if (!_compiler.assemblyItems(_contract) && !_compiler.runtimeAssemblyItems(_contract))
		return gasEstimates;
	if (sof::AssemblyItems const* items = _compiler.assemblyItems(_contract))
	{
		Gas gas = GasEstimator::functionalEstimation(*items);
		u256 bytecodeSize(_compiler.runtimeObject(_contract).bytecode.size());
		Json::Value creationGas(Json::arrayValue);
		creationGas[0] = gasToJson(gas);
		creationGas[1] = gasToJson(bytecodeSize * sof::c_createDataGas);
		gasEstimates["creation"] = creationGas;
	}
	if (sof::AssemblyItems const* items = _compiler.runtimeAssemblyItems(_contract))
	{
		ContractDefinition const& contract = _compiler.contractDefinition(_contract);
		Json::Value externalFunctions(Json::objectValue);
		for (auto it: contract.interfaceFunctions())
		{
			string sig = it.second->externalSignature();
			externalFunctions[sig] = gasToJson(GasEstimator::functionalEstimation(*items, sig));
		}
		if (contract.fallbackFunction())
			externalFunctions[""] = gasToJson(GasEstimator::functionalEstimation(*items, "INVALID"));
		gasEstimates["external"] = externalFunctions;
		Json::Value internalFunctions(Json::objectValue);
		for (auto const& it: contract.definedFunctions())
		{
			if (it->isPartOfExternalInterface() || it->isConstructor())
				continue;
			size_t entry = _compiler.functionEntryPoint(_contract, *it);
			GasEstimator::GasConsumption gas = GasEstimator::GasConsumption::infinite();
			if (entry > 0)
				gas = GasEstimator::functionalEstimation(*items, entry, *it);
			FunctionType type(*it);
			string sig = it->name() + "(";
			auto end = type.parameterTypes().end();
			for (auto it = type.parameterTypes().begin(); it != end; ++it)
				sig += (*it)->toString() + (it + 1 == end ? "" : ",");
			sig += ")";
			internalFunctions[sig] = gasToJson(gas);
		}
		gasEstimates["internal"] = internalFunctions;
	}
	return gasEstimates;
}

string compile(string _input, bool _optimize)
{
	StringMap sources;
	sources[""] = _input;

	Json::Value output(Json::objectValue);
	Json::Value errors(Json::arrayValue);
	CompilerStack compiler;
	bool success = false;
	try
	{
		bool succ = compiler.compile(_input, _optimize);
		for (auto const& error: compiler.errors())
			errors.append(formatError(
				*error,
				(dynamic_pointer_cast<Warning const>(error)) ? "Warning" : "Error",
				compiler
			));
		success = succ; // keep success false on exception
	}
	catch (ParserError const& exception)
	{
		errors.append(formatError(exception, "Parser error", compiler));
	}
	catch (DeclarationError const& exception)
	{
		errors.append(formatError(exception, "Declaration error", compiler));
	}
	catch (TypeError const& exception)
	{
		errors.append(formatError(exception, "Type error", compiler));
	}
	catch (CompilerError const& exception)
	{
		errors.append(formatError(exception, "Compiler error", compiler));
	}
	catch (InternalCompilerError const& exception)
	{
		errors.append(formatError(exception, "Internal compiler error", compiler));
	}
	catch (DocstringParsingError const& exception)
	{
		errors.append(formatError(exception, "Documentation parsing error", compiler));
	}
	catch (Exception const& exception)
	{
		errors.append("Exception during compilation: " + boost::diagnostic_information(exception));
	}
	catch (...)
	{
		errors.append("Unknown exception during compilation.");
	}

	if (errors.size() > 0)
		output["errors"] = errors;

	if (success)
	{
		output["contracts"] = Json::Value(Json::objectValue);
		for (string const& contractName: compiler.contractNames())
		{
			Json::Value contractData(Json::objectValue);
			contractData["polynomial_interface"] = compiler.polynomialInterface(contractName);
			contractData["interface"] = compiler.interface(contractName);
			contractData["bytecode"] = compiler.object(contractName).toHex();
			contractData["runtimeBytecode"] = compiler.runtimeObject(contractName).toHex();
			contractData["opcodes"] = sof::disassemble(compiler.object(contractName).bytecode);
			contractData["functionHashes"] = functionHashes(compiler.contractDefinition(contractName));
			contractData["gasEstimates"] = estimateGas(compiler, contractName);
			ostringstream unused;
			contractData["assembly"] = compiler.streamAssembly(unused, contractName, sources, true);
			output["contracts"][contractName] = contractData;
		}

		output["sources"] = Json::Value(Json::objectValue);
		output["sources"][""] = Json::Value(Json::objectValue);
		output["sources"][""]["AST"] = ASTJsonConverter(compiler.ast("")).json();
	}

	return Json::FastWriter().write(output);
}

static string outputBuffer;

extern "C"
{
extern char const* version()
{
	return VersionString.c_str();
}
extern char const* compileJSON(char const* _input, bool _optimize)
{
	outputBuffer = compile(_input, _optimize);
	return outputBuffer.c_str();
}
}
