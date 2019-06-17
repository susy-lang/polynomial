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

#include <libpolynomial/formal/SSAVariable.h>

#include <libpolynomial/formal/SymbolicIntVariable.h>

#include <libpolynomial/ast/AST.h>

using namespace std;
using namespace dev;
using namespace dev::polynomial;

SSAVariable::SSAVariable(
	Declaration const* _decl,
	smt::SolverInterface& _interface
)
{
	resetIndex();

	if (dynamic_cast<IntegerType const*>(_decl->type().get()))
		m_symbolicVar = make_shared<SymbolicIntVariable>(_decl, _interface);
	else
	{
		polAssert(false, "");
	}
}

bool SSAVariable::supportedType(Type const* _decl)
{
	return dynamic_cast<IntegerType const*>(_decl);
}

void SSAVariable::resetIndex()
{
	m_currentSequenceCounter = 0;
	m_nextFreeSequenceCounter.reset (new int);
	*m_nextFreeSequenceCounter = 1;
}

int SSAVariable::index() const
{
	return m_currentSequenceCounter;
}

int SSAVariable::next() const
{
	return *m_nextFreeSequenceCounter;
}

void SSAVariable::setZeroValue()
{
	m_symbolicVar->setZeroValue(index());
}

void SSAVariable::setUnknownValue()
{
	m_symbolicVar->setUnknownValue(index());
}
