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
 * @date 2017
 * Pull in some identifiers from the polynomial::assembly namespace.
 */

#pragma once

#include <libpolynomial/inlineasm/AsmDataForward.h>

namespace dev
{
namespace julia
{

using Instruction = polynomial::assembly::Instruction;
using Literal = polynomial::assembly::Literal;
using Label = polynomial::assembly::Label;
using StackAssignment = polynomial::assembly::StackAssignment;
using Identifier = polynomial::assembly::Identifier;
using Assignment = polynomial::assembly::Assignment;
using VariableDeclaration = polynomial::assembly::VariableDeclaration;
using FunctionalInstruction = polynomial::assembly::FunctionalInstruction;
using FunctionDefinition = polynomial::assembly::FunctionDefinition;
using FunctionCall = polynomial::assembly::FunctionCall;
using If = polynomial::assembly::If;
using Case = polynomial::assembly::Case;
using Switch = polynomial::assembly::Switch;
using ForLoop = polynomial::assembly::ForLoop;
using ExpressionStatement = polynomial::assembly::ExpressionStatement;
using Block = polynomial::assembly::Block;

using TypedName = polynomial::assembly::TypedName;

using Expression = boost::variant<FunctionalInstruction, FunctionCall, Identifier, Literal>;
using Statement = boost::variant<ExpressionStatement, Instruction, Label, StackAssignment, Assignment, VariableDeclaration, FunctionDefinition, If, Switch, ForLoop, Block>;

}
}
