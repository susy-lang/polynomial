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
using namespace dev::polynomial::smt;

set<VariableDeclaration const*> VariableUsage::touchedVariables(ASTNode const& _node, vector<CallableDeclaration const*> const& _outerCallstack)
{
	m_touchedVariables.clear();
	m_callStack.clear();
	m_callStack += _outerCallstack;
	m_lastCall = m_callStack.back();
	_node.accept(*this);
	return m_touchedVariables;
}

void VariableUsage::endVisit(Identifier const& _identifier)
{
	if (_identifier.annotation().lValueRequested)
		checkIdentifier(_identifier);
}

void VariableUsage::endVisit(IndexAccess const& _indexAccess)
{
	if (_indexAccess.annotation().lValueRequested)
	{
		/// identifier.annotation().lValueRequested == false, that's why we
		/// need to check that before.
		auto identifier = dynamic_cast<Identifier const*>(SMTChecker::leftmostBase(_indexAccess));
		if (identifier)
			checkIdentifier(*identifier);
	}
}

void VariableUsage::endVisit(FunctionCall const& _funCall)
{
	if (auto const& funDef = SMTChecker::inlinedFunctionCallToDefinition(_funCall))
		if (find(m_callStack.begin(), m_callStack.end(), funDef) == m_callStack.end())
			funDef->accept(*this);
}

bool VariableUsage::visit(FunctionDefinition const& _function)
{
	m_callStack.push_back(&_function);
	return true;
}

void VariableUsage::endVisit(FunctionDefinition const&)
{
	polAssert(!m_callStack.empty(), "");
	m_callStack.pop_back();
}

void VariableUsage::endVisit(ModifierInvocation const& _modifierInv)
{
	auto const& modifierDef = dynamic_cast<ModifierDefinition const*>(_modifierInv.name()->annotation().referencedDeclaration);
	if (modifierDef)
		modifierDef->accept(*this);
}

void VariableUsage::endVisit(PlaceholderStatement const&)
{
	polAssert(!m_callStack.empty(), "");
	FunctionDefinition const* funDef = nullptr;
	for (auto it = m_callStack.rbegin(); it != m_callStack.rend() && !funDef; ++it)
		funDef = dynamic_cast<FunctionDefinition const*>(*it);
	polAssert(funDef, "");
	if (funDef->isImplemented())
		funDef->body().accept(*this);
}

void VariableUsage::checkIdentifier(Identifier const& _identifier)
{
	Declaration const* declaration = _identifier.annotation().referencedDeclaration;
	polAssert(declaration, "");
	if (VariableDeclaration const* varDecl = dynamic_cast<VariableDeclaration const*>(declaration))
	{
		polAssert(m_lastCall, "");
		if (!varDecl->isLocalVariable() || varDecl->functionOrModifierDefinition() == m_lastCall)
			m_touchedVariables.insert(varDecl);
	}
}
