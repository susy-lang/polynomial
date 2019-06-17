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

#include <libpolynomial/analysis/PostTypeChecker.h>
#include <libpolynomial/ast/AST.h>
#include <libpolynomial/analysis/SemVerHandler.h>
#include <libpolynomial/interface/ErrorReporter.h>
#include <libpolynomial/interface/Version.h>

#include <boost/range/adaptor/map.hpp>

#include <memory>

using namespace std;
using namespace dev;
using namespace dev::polynomial;


bool PostTypeChecker::check(ASTNode const& _astRoot)
{
	_astRoot.accept(*this);
	return Error::containsOnlyWarnings(m_errorReporter.errors());
}

bool PostTypeChecker::visit(ContractDefinition const&)
{
	polAssert(!m_currentConstVariable, "");
	polAssert(m_constVariableDependencies.empty(), "");
	return true;
}

void PostTypeChecker::endVisit(ContractDefinition const&)
{
	polAssert(!m_currentConstVariable, "");
	for (auto declaration: m_constVariables)
		if (auto identifier = findCycle(declaration))
			m_errorReporter.typeError(
				declaration->location(),
				"The value of the constant " + declaration->name() +
				" has a cyclic dependency via " + identifier->name() + "."
			);

	m_constVariables.clear();
	m_constVariableDependencies.clear();
}

bool PostTypeChecker::visit(VariableDeclaration const& _variable)
{
	polAssert(!m_currentConstVariable, "");
	if (_variable.isConstant())
	{
		m_currentConstVariable = &_variable;
		m_constVariables.push_back(&_variable);
	}
	return true;
}

void PostTypeChecker::endVisit(VariableDeclaration const& _variable)
{
	if (_variable.isConstant())
	{
		polAssert(m_currentConstVariable == &_variable, "");
		m_currentConstVariable = nullptr;
	}
}

bool PostTypeChecker::visit(Identifier const& _identifier)
{
	if (m_currentConstVariable)
		if (auto var = dynamic_cast<VariableDeclaration const*>(_identifier.annotation().referencedDeclaration))
			if (var->isConstant())
				m_constVariableDependencies[m_currentConstVariable].insert(var);
	return true;
}

VariableDeclaration const* PostTypeChecker::findCycle(
	VariableDeclaration const* _startingFrom,
	set<VariableDeclaration const*> const& _seen
)
{
	if (_seen.count(_startingFrom))
		return _startingFrom;
	else if (m_constVariableDependencies.count(_startingFrom))
	{
		set<VariableDeclaration const*> seen(_seen);
		seen.insert(_startingFrom);
		for (auto v: m_constVariableDependencies[_startingFrom])
			if (findCycle(v, seen))
				return v;
	}
	return nullptr;
}