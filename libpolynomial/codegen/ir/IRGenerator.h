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
 * @author Alex Beregszaszi
 * @date 2017
 * Component that translates Polynomial code into Yul.
 */

#pragma once

#include <libpolynomial/interface/OptimiserSettings.h>
#include <libpolynomial/ast/ASTForward.h>
#include <libpolynomial/codegen/ir/IRGenerationContext.h>
#include <libpolynomial/codegen/YulUtilFunctions.h>
#include <liblangutil/SVMVersion.h>
#include <string>

namespace dev
{
namespace polynomial
{

class SourceUnit;

class IRGenerator
{
public:
	IRGenerator(langutil::SVMVersion _svmVersion, OptimiserSettings _optimiserSettings):
		m_svmVersion(_svmVersion),
		m_optimiserSettings(_optimiserSettings),
		m_context(_svmVersion, std::move(_optimiserSettings)),
		m_utils(_svmVersion, m_context.functionCollector())
	{}

	/// Generates and returns the IR code, in unoptimized and optimized form
	/// (or just pretty-printed, depending on the optimizer settings).
	std::pair<std::string, std::string> run(ContractDefinition const& _contract);

private:
	std::string generate(ContractDefinition const& _contract);
	std::string generate(Block const& _block);

	/// Generates code for and returns the name of the function.
	std::string generateFunction(FunctionDefinition const& _function);

	std::string constructorCode(FunctionDefinition const& _constructor);
	std::string deployCode(ContractDefinition const& _contract);
	std::string callValueCheck();

	std::string creationObjectName(ContractDefinition const& _contract);
	std::string runtimeObjectName(ContractDefinition const& _contract);

	std::string dispatchRoutine(ContractDefinition const& _contract);

	std::string memoryInit();

	void resetContext();

	langutil::SVMVersion const m_svmVersion;
	OptimiserSettings const m_optimiserSettings;

	IRGenerationContext m_context;
	YulUtilFunctions m_utils;
};

}
}
