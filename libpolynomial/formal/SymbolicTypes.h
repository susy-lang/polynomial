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

#pragma once

#include <libpolynomial/formal/SolverInterface.h>
#include <libpolynomial/formal/SymbolicVariables.h>
#include <libpolynomial/ast/AST.h>
#include <libpolynomial/ast/Types.h>

namespace dev
{
namespace polynomial
{
namespace smt
{

/// Returns the SMT sort that models the Polynomial type _type.
SortPointer smtSort(polynomial::Type const& _type);
std::vector<SortPointer> smtSort(std::vector<polynomial::TypePointer> const& _types);
/// Returns the SMT kind that models the Polynomial type type category _category.
Kind smtKind(polynomial::Type::Category _category);

/// Returns true if type is fully supported (declaration and operations).
bool isSupportedType(polynomial::Type::Category _category);
bool isSupportedType(polynomial::Type const& _type);
/// Returns true if type is partially supported (declaration).
bool isSupportedTypeDeclaration(polynomial::Type::Category _category);
bool isSupportedTypeDeclaration(polynomial::Type const& _type);

bool isInteger(polynomial::Type::Category _category);
bool isRational(polynomial::Type::Category _category);
bool isFixedBytes(polynomial::Type::Category _category);
bool isAddress(polynomial::Type::Category _category);
bool isContract(polynomial::Type::Category _category);
bool isEnum(polynomial::Type::Category _category);
bool isNumber(polynomial::Type::Category _category);
bool isBool(polynomial::Type::Category _category);
bool isFunction(polynomial::Type::Category _category);
bool isMapping(polynomial::Type::Category _category);
bool isArray(polynomial::Type::Category _category);
bool isTuple(polynomial::Type::Category _category);

/// Returns a new symbolic variable, according to _type.
/// Also returns whether the type is abstract or not,
/// which is true for unsupported types.
std::pair<bool, std::shared_ptr<SymbolicVariable>> newSymbolicVariable(polynomial::Type const& _type, std::string const& _uniqueName, SolverInterface& _solver);

Expression minValue(polynomial::IntegerType const& _type);
Expression maxValue(polynomial::IntegerType const& _type);

void setSymbolicZeroValue(SymbolicVariable const& _variable, SolverInterface& _interface);
void setSymbolicZeroValue(Expression _expr, polynomial::TypePointer const& _type, SolverInterface& _interface);
void setSymbolicUnknownValue(SymbolicVariable const& _variable, SolverInterface& _interface);
void setSymbolicUnknownValue(Expression _expr, polynomial::TypePointer const& _type, SolverInterface& _interface);

}
}
}
