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
 * Optimisation stage that replaces expressions known to be the current value of a variable
 * in scope by a reference to that variable.
 */

#pragma once

#include <libyul/optimiser/DataFlowAnalyzer.h>

namespace yul
{

struct Dialect;

/**
 * Optimisation stage that replaces expressions known to be the current value of a variable
 * in scope by a reference to that variable.
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter.
 */
class CommonSubexpressionEliminator: public DataFlowAnalyzer
{
public:
	CommonSubexpressionEliminator(Dialect const& _dialect): DataFlowAnalyzer(_dialect) {}

protected:
	using ASTModifier::visit;
	void visit(Expression& _e) override;
};

}
