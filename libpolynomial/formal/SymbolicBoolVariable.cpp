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

#include <libpolynomial/formal/SymbolicBoolVariable.h>

#include <libpolynomial/ast/AST.h>

using namespace std;
using namespace dev;
using namespace dev::polynomial;

SymbolicBoolVariable::SymbolicBoolVariable(
	Declaration const& _decl,
	smt::SolverInterface&_interface
):
	SymbolicVariable(_decl, _interface)
{
	polAssert(m_declaration.type()->category() == Type::Category::Bool, "");
}

smt::Expression SymbolicBoolVariable::valueAtSequence(int _seq) const
{
	return m_interface.newBool(uniqueSymbol(_seq));
}

void SymbolicBoolVariable::setZeroValue(int _seq)
{
	m_interface.addAssertion(valueAtSequence(_seq) == smt::Expression(false));
}

void SymbolicBoolVariable::setUnknownValue(int)
{
}