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

#include <unordered_map>
#include <set>

namespace dev
{
namespace polynomial
{
namespace smt
{

/**
 * Stores the context of the SMT encoding.
 */
class EncodingContext
{
public:
	EncodingContext(SolverInterface& _solver);

	/// Resets the entire context.
	void reset();

	/// Methods related to variables.
	//@{
	/// @returns the symbolic representation of a program variable.
	std::shared_ptr<SymbolicVariable> variable(polynomial::VariableDeclaration const& _varDecl);
	/// @returns all symbolic variables.
	std::unordered_map<polynomial::VariableDeclaration const*, std::shared_ptr<SymbolicVariable>> const& variables() const { return m_variables; }

	/// Creates a symbolic variable and
	/// @returns true if a variable's type is not supported and is therefore abstract.
	bool createVariable(polynomial::VariableDeclaration const& _varDecl);
	/// @returns true if variable was created.
	bool knownVariable(polynomial::VariableDeclaration const& _varDecl);

	/// Resets a specific variable.
	void resetVariable(polynomial::VariableDeclaration const& _variable);
	/// Resets a set of variables.
	void resetVariables(std::set<polynomial::VariableDeclaration const*> const& _variables);
	/// Resets variables according to a predicate.
	void resetVariables(std::function<bool(polynomial::VariableDeclaration const&)> const& _filter);
	///Resets all variables.
	void resetAllVariables();

	/// Allocates a new index for the declaration, updates the current
	/// index to this value and returns the expression.
	Expression newValue(polynomial::VariableDeclaration const& _decl);
	/// Sets the value of the declaration to zero.
	void setZeroValue(polynomial::VariableDeclaration const& _decl);
	void setZeroValue(SymbolicVariable& _variable);
	/// Resets the variable to an unknown value (in its range).
	void setUnknownValue(polynomial::VariableDeclaration const& decl);
	void setUnknownValue(SymbolicVariable& _variable);
	//@}

	/// Methods related to expressions.
	//@{
	/// @returns the symbolic representation of an AST node expression.
	std::shared_ptr<SymbolicVariable> expression(polynomial::Expression const& _e);
	/// @returns all symbolic expressions.
	std::unordered_map<polynomial::Expression const*, std::shared_ptr<SymbolicVariable>> const& expressions() const { return m_expressions; }

	/// Creates the expression (value can be arbitrary).
	/// @returns true if type is not supported.
	bool createExpression(polynomial::Expression const& _e, std::shared_ptr<SymbolicVariable> _symbExpr = nullptr);
	/// Checks if expression was created.
	bool knownExpression(polynomial::Expression const& _e) const;
	//@}

	/// Methods related to global variables and functions.
	//@{
	/// Global variables and functions.
	std::shared_ptr<SymbolicVariable> globalSymbol(std::string const& _name);
	/// @returns all symbolic variables.
	std::unordered_map<std::string, std::shared_ptr<SymbolicVariable>> const& globalSymbols() const { return m_globalContext; }
	/// Defines a new global variable or function
	/// and @returns true if type was abstracted.
	bool createGlobalSymbol(std::string const& _name, polynomial::Expression const& _expr);
	/// Checks if special variable or function was seen.
	bool knownGlobalSymbol(std::string const& _var) const;
	//@}

	/// Blockchain related methods.
	//@{
	/// Value of `this` address.
	Expression thisAddress();
	/// @returns the symbolic balance of address `this`.
	Expression balance();
	/// @returns the symbolic balance of an address.
	Expression balance(Expression _address);
	/// Transfer _value from _from to _to.
	void transfer(Expression _from, Expression _to, Expression _value);
	//@}

private:
	/// Adds _value to _account's balance.
	void addBalance(Expression _account, Expression _value);

	SolverInterface& m_solver;

	/// Symbolic variables.
	std::unordered_map<polynomial::VariableDeclaration const*, std::shared_ptr<SymbolicVariable>> m_variables;

	/// Symbolic expressions.
	std::unordered_map<polynomial::Expression const*, std::shared_ptr<SymbolicVariable>> m_expressions;

	/// Symbolic representation of global symbols including
	/// variables and functions.
	std::unordered_map<std::string, std::shared_ptr<smt::SymbolicVariable>> m_globalContext;

	/// Symbolic `this` address.
	std::unique_ptr<SymbolicAddressVariable> m_thisAddress;

	/// Symbolic balances.
	std::unique_ptr<SymbolicVariable> m_balances;
};

}
}
}
