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
 * Polynomial inline assembly parser.
 */

#pragma once

#include <memory>
#include <vector>
#include <libpolynomial/inlineasm/AsmData.h>
#include <libpolynomial/parsing/ParserBase.h>

namespace dev
{
namespace polynomial
{
namespace assembly
{

class Parser: public ParserBase
{
public:
	explicit Parser(ErrorReporter& _errorReporter, bool _julia = false): ParserBase(_errorReporter), m_julia(_julia) {}

	/// Parses an inline assembly block starting with `{` and ending with `}`.
	/// @returns an empty shared pointer on error.
	std::shared_ptr<Block> parse(std::shared_ptr<Scanner> const& _scanner);

protected:
	/// Creates an inline assembly node with the given source location.
	template <class T> T createWithLocation(SourceLocation const& _loc = SourceLocation()) const
	{
		T r;
		r.location = _loc;
		if (r.location.isEmpty())
		{
			r.location.start = position();
			r.location.end = endPosition();
		}
		if (!r.location.sourceName)
			r.location.sourceName = sourceName();
		return r;
	}
	SourceLocation location() const { return SourceLocation(position(), endPosition(), sourceName()); }

	Block parseBlock();
	Statement parseStatement();
	Case parseCase();
	ForLoop parseForLoop();
	/// Parses a functional expression that has to push exactly one stack element
	Statement parseExpression();
	static std::map<std::string, dev::polynomial::Instruction> const& instructions();
	static std::map<dev::polynomial::Instruction, std::string> const& instructionNames();
	Statement parseElementaryOperation(bool _onlySinglePusher = false);
	VariableDeclaration parseVariableDeclaration();
	FunctionDefinition parseFunctionDefinition();
	Statement parseCall(Statement&& _instruction);
	TypedName parseTypedName();
	std::string expectAsmIdentifier();

	static bool isValidNumberLiteral(std::string const& _literal);

private:
	bool m_julia = false;
};

}
}
}
