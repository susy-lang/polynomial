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
 * @author Christian <c@sofdev.com>
 * @date 2016
 * Code-generating part of inline assembly.
 */

#pragma once

#include <libpolynomial/inlineasm/AsmAnalysis.h>

#include <functional>

namespace dev
{
namespace sof
{
class Assembly;
}
namespace polynomial
{
namespace assembly
{
struct Block;

class CodeGenerator
{
public:
	/// Performs code generation and appends generated to _assembly.
	static void assemble(
		Block const& _parsedData,
		AsmAnalysisInfo& _analysisInfo,
		sof::Assembly& _assembly,
		yul::ExternalIdentifierAccess const& _identifierAccess = yul::ExternalIdentifierAccess(),
		bool _useNamedLabelsForFunctions = false
	);
};

}
}
}
