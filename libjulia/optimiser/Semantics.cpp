/*(
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
 * Specific AST walkers that collect semantical facts.
 */

#include <libjulia/optimiser/Semantics.h>

#include <libpolynomial/inlineasm/AsmData.h>

#include <libsvmasm/SemanticInformation.h>

#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace dev::julia;

MovableChecker::MovableChecker(Expression const& _expression)
{
	visit(_expression);
}

void MovableChecker::operator()(Identifier const& _identifier)
{
	ASTWalker::operator()(_identifier);
	m_variableReferences.insert(_identifier.name);
}

void MovableChecker::operator()(FunctionalInstruction const& _instr)
{
	if (!sof::SemanticInformation::movable(_instr.instruction))
		m_movable = false;
	else
		ASTWalker::operator()(_instr);
}

void MovableChecker::operator()(FunctionCall const&)
{
	m_movable = false;
}

void MovableChecker::visit(Statement const&)
{
	polAssert(false, "Movability for statement requested.");
}
