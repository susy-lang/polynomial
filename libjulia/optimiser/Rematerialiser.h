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
 * Optimisation stage that replaces variables by their most recently assigned expressions.
 */

#pragma once

#include <libjulia/optimiser/DataFlowAnalyzer.h>

#include <string>
#include <map>
#include <set>

namespace dev
{
namespace julia
{

/**
 * Optimisation stage that replaces variables by their most recently assigned expressions.
 *
 * Prerequisite: Disambiguator
 */
class Rematerialiser: public DataFlowAnalyzer
{
protected:
	using ASTModifier::visit;
	virtual void visit(Expression& _e) override;

};

}
}