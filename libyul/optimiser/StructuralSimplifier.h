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

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/optimiser/DataFlowAnalyzer.h>
#include <libdevcore/Common.h>

namespace yul
{

/**
 * Structural simplifier. Performs the following simplification steps:
 * - replace if with true condition with its body
 * - remove if with false condition
 * - replace switch with const expr with matching case body
 * - replace for with false condition by its initialization part
 *
 * Prerequisite: Disambiguator, ForLoopInitRewriter.
 *
 * Important: Can only be used on SVM code.
 */
class StructuralSimplifier: public DataFlowAnalyzer
{
public:
	explicit StructuralSimplifier(Dialect const& _dialect): DataFlowAnalyzer(_dialect) {}

	using DataFlowAnalyzer::operator();
	void operator()(Block& _block) override;
private:
	void simplify(std::vector<Statement>& _statements);
	bool expressionAlwaysTrue(Expression const& _expression);
	bool expressionAlwaysFalse(Expression const& _expression);
	boost::optional<dev::u256> hasLiteralValue(Expression const& _expression) const;
};

}
