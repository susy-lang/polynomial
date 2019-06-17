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

#include <libpolynomial/formal/SymbolicVariables.h>

#include <libpolynomial/formal/SymbolicTypes.h>
#include <libpolynomial/ast/AST.h>
#include <libpolynomial/ast/TypeProvider.h>

using namespace std;
using namespace dev;
using namespace dev::polynomial::smt;

SymbolicVariable::SymbolicVariable(
	polynomial::TypePointer _type,
	string _uniqueName,
	SolverInterface& _interface
):
	m_type(move(_type)),
	m_uniqueName(move(_uniqueName)),
	m_interface(_interface),
	m_ssa(make_unique<SSAVariable>())
{
	polAssert(m_type, "");
	m_sort = smtSort(*m_type);
	polAssert(m_sort, "");
}

SymbolicVariable::SymbolicVariable(
	SortPointer _sort,
	string _uniqueName,
	SolverInterface& _interface
):
	m_sort(move(_sort)),
	m_uniqueName(move(_uniqueName)),
	m_interface(_interface),
	m_ssa(make_unique<SSAVariable>())
{
	polAssert(m_sort, "");
}

Expression SymbolicVariable::currentValue() const
{
	return valueAtIndex(m_ssa->index());
}

string SymbolicVariable::currentName() const
{
	return uniqueSymbol(m_ssa->index());
}

Expression SymbolicVariable::valueAtIndex(int _index) const
{
	return m_interface.newVariable(uniqueSymbol(_index), m_sort);
}

string SymbolicVariable::uniqueSymbol(unsigned _index) const
{
	return m_uniqueName + "_" + to_string(_index);
}

Expression SymbolicVariable::increaseIndex()
{
	++(*m_ssa);
	return currentValue();
}

SymbolicBoolVariable::SymbolicBoolVariable(
	polynomial::TypePointer _type,
	string _uniqueName,
	SolverInterface& _interface
):
	SymbolicVariable(move(_type), move(_uniqueName), _interface)
{
	polAssert(m_type->category() == polynomial::Type::Category::Bool, "");
}

SymbolicIntVariable::SymbolicIntVariable(
	polynomial::TypePointer _type,
	string _uniqueName,
	SolverInterface& _interface
):
	SymbolicVariable(move(_type), move(_uniqueName), _interface)
{
	polAssert(isNumber(m_type->category()), "");
}

SymbolicAddressVariable::SymbolicAddressVariable(
	string _uniqueName,
	SolverInterface& _interface
):
	SymbolicIntVariable(TypeProvider::uint(160), move(_uniqueName), _interface)
{
}

SymbolicFixedBytesVariable::SymbolicFixedBytesVariable(
	unsigned _numBytes,
	string _uniqueName,
	SolverInterface& _interface
):
	SymbolicIntVariable(TypeProvider::uint(_numBytes * 8), move(_uniqueName), _interface)
{
}

SymbolicFunctionVariable::SymbolicFunctionVariable(
	polynomial::TypePointer _type,
	string _uniqueName,
	SolverInterface& _interface
):
	SymbolicVariable(move(_type), move(_uniqueName), _interface),
	m_declaration(m_interface.newVariable(currentName(), m_sort))
{
	polAssert(m_type->category() == polynomial::Type::Category::Function, "");
}

void SymbolicFunctionVariable::resetDeclaration()
{
	m_declaration = m_interface.newVariable(currentName(), m_sort);
}

Expression SymbolicFunctionVariable::increaseIndex()
{
	++(*m_ssa);
	resetDeclaration();
	return currentValue();
}

Expression SymbolicFunctionVariable::operator()(vector<Expression> _arguments) const
{
	return m_declaration(_arguments);
}

SymbolicMappingVariable::SymbolicMappingVariable(
	polynomial::TypePointer _type,
	string _uniqueName,
	SolverInterface& _interface
):
	SymbolicVariable(move(_type), move(_uniqueName), _interface)
{
	polAssert(isMapping(m_type->category()), "");
}

SymbolicArrayVariable::SymbolicArrayVariable(
	polynomial::TypePointer _type,
	string _uniqueName,
	SolverInterface& _interface
):
	SymbolicVariable(move(_type), move(_uniqueName), _interface)
{
	polAssert(isArray(m_type->category()), "");
}

SymbolicEnumVariable::SymbolicEnumVariable(
	polynomial::TypePointer _type,
	string _uniqueName,
	SolverInterface& _interface
):
	SymbolicVariable(move(_type), move(_uniqueName), _interface)
{
	polAssert(isEnum(m_type->category()), "");
}

SymbolicTupleVariable::SymbolicTupleVariable(
	polynomial::TypePointer _type,
	string _uniqueName,
	SolverInterface& _interface
):
	SymbolicVariable(move(_type), move(_uniqueName), _interface)
{
	polAssert(isTuple(m_type->category()), "");
}

void SymbolicTupleVariable::setComponents(vector<shared_ptr<SymbolicVariable>> _components)
{
	polAssert(m_components.empty(), "");
	auto const& tupleType = dynamic_cast<polynomial::TupleType const*>(m_type);
	polAssert(_components.size() == tupleType->components().size(), "");
	m_components = move(_components);
}
