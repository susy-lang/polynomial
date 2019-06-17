/*
	This file is part of cpp-sophon.

	cpp-sophon is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-sophon is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MSRCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-sophon.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@sofdev.com>
 * @date 2015
 * Evaluator for types of constant expressions.
 */

#pragma once

#include <libpolynomial/ast/ASTVisitor.h>

namespace dev
{
namespace polynomial
{

class TypeChecker;

/**
 * Small drop-in replacement for TypeChecker to evaluate simple expressions of integer constants.
 */
class ConstantEvaluator: private ASTConstVisitor
{
public:
	ConstantEvaluator(Expression const& _expr) { _expr.accept(*this); }

private:
	virtual void endVisit(BinaryOperation const& _operation);
	virtual void endVisit(UnaryOperation const& _operation);
	virtual void endVisit(Literal const& _literal);

};

}
}