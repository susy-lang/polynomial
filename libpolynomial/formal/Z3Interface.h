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

#include <boost/noncopyable.hpp>

#include <z3++.h>

namespace dev
{
namespace polynomial
{
namespace smt
{

class Z3Interface: public SolverInterface, public boost::noncopyable
{
public:
	Z3Interface();

	void reset() override;

	void push() override;
	void pop() override;

	void declareFunction(std::string _name, Sort _domain, Sort _codomain) override;
	void declareInteger(std::string _name) override;
	void declareBool(std::string _name) override;

	void addAssertion(Expression const& _expr) override;
	std::pair<CheckResult, std::vector<std::string>> check(std::vector<Expression> const& _expressionsToEvaluate) override;

private:
	z3::expr toZ3Expr(Expression const& _expr);
	z3::sort z3Sort(smt::Sort _sort);

	z3::context m_context;
	z3::solver m_solver;
	std::map<std::string, z3::expr> m_constants;
	std::map<std::string, z3::func_decl> m_functions;
};

}
}
}
