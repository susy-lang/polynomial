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

#include <libpolynomial/formal/VariableUsage.h>

#include <libpolynomial/formal/SMTChecker.h>

#include <algorithm>

using namespace std;
using namespace dev;
using namespace dev::polynomial;

void VariableUsage::endVisit(Identifier const& _identifier)
{
	Declaration const* declaration = _identifier.annotation().referencedDeclaration;
	polAssert(declaration, "");
	if (VariableDeclaration const* varDecl = dynamic_cast<VariableDeclaration const*>(declaration))
		if (_identifier.annotation().lValueRequested)
			m_touchedVariables.insert(varDecl);
}

void VariableUsage::endVisit(FunctionCall const& _funCall)
{
	if (auto const& funDef = SMTChecker::inlinedFunctionCallToDefinition(_funCall))
		if (find(m_functionPath.begin(), m_functionPath.end(), funDef) == m_functionPath.end())
			funDef->accept(*this);
}

bool VariableUsage::visit(FunctionDefinition const& _function)
{
	m_functionPath.push_back(&_function);
	return true;
}

void VariableUsage::endVisit(FunctionDefinition const&)
{
	polAssert(!m_functionPath.empty(), "");
	m_functionPath.pop_back();
}

void VariableUsage::endVisit(ModifierInvocation const& _modifierInv)
{
	auto const& modifierDef = dynamic_cast<ModifierDefinition const*>(_modifierInv.name()->annotation().referencedDeclaration);
	if (modifierDef)
		modifierDef->accept(*this);
}

void VariableUsage::endVisit(PlaceholderStatement const&)
{
	polAssert(!m_functionPath.empty(), "");
	FunctionDefinition const* function = m_functionPath.back();
	polAssert(function, "");
	if (function->isImplemented())
		function->body().accept(*this);
}

set<VariableDeclaration const*> VariableUsage::touchedVariables(ASTNode const& _node, vector<FunctionDefinition const*> const& _outerCallstack)
{
	m_touchedVariables.clear();
	m_functionPath.clear();
	m_functionPath += _outerCallstack;
	_node.accept(*this);
	return m_touchedVariables;
}
