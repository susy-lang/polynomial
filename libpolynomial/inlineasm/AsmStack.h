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
 * Full-stack Polynomial inline assember.
 */

#pragma once

#include <string>
#include <functional>
#include <libpolynomial/interface/Exceptions.h>
#include <libpolynomial/inlineasm/AsmCodeGen.h>

namespace dev
{
namespace sof
{
class Assembly;
}
namespace polynomial
{
class Scanner;
namespace assembly
{
struct Block;

class InlineAssemblyStack
{
public:
	/// Parse the given inline assembly chunk starting with `{` and ending with the corresponding `}`.
	/// @return false or error.
	bool parse(std::shared_ptr<Scanner> const& _scanner);
	sof::Assembly assemble();

	/// Parse and assemble a string in one run - for use in Polynomial code generation itself.
	bool parseAndAssemble(
		std::string const& _input,
		sof::Assembly& _assembly,
		CodeGenerator::IdentifierAccess const& _identifierAccess = CodeGenerator::IdentifierAccess()
	);

	ErrorList const& errors() const { return m_errors; }

private:
	std::shared_ptr<Block> m_parserResult;
	ErrorList m_errors;
};

}
}
}
