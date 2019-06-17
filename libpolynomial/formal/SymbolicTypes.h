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

/// So far int, bool and address are supported.
/// Returns true if type is supported.
bool isSupportedType(Type::Category _category);
bool isSupportedType(Type const& _type);

bool isInteger(Type::Category _category);
bool isRational(Type::Category _category);
bool isFixedBytes(Type::Category _category);
bool isAddress(Type::Category _category);
bool isNumber(Type::Category _category);
bool isBool(Type::Category _category);
bool isFunction(Type::Category _category);

/// Returns a new symbolic variable, according to _type.
/// Also returns whether the type is abstract or not,
/// which is true for unsupported types.
std::pair<bool, std::shared_ptr<SymbolicVariable>> newSymbolicVariable(Type const& _type, std::string const& _uniqueName, smt::SolverInterface& _solver);

smt::Expression minValue(IntegerType const& _type);
smt::Expression maxValue(IntegerType const& _type);

}
}
