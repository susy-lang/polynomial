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

#include <libpolynomial/ast/ASTVisitor.h>

using namespace std;
using namespace dev;
using namespace dev::polynomial;

VariableUsage::VariableUsage(ASTNode const& _node)
{
	auto nodeFun = [&](ASTNode const& n) -> bool
	{
		if (Identifier const* identifier = dynamic_cast<decltype(identifier)>(&n))
		{
			Declaration const* declaration = identifier->annotation().referencedDeclaration;
			polAssert(declaration, "");
			if (VariableDeclaration const* varDecl = dynamic_cast<VariableDeclaration const*>(declaration))
				if (
					varDecl->isLocalVariable() &&
					identifier->annotation().lValueRequested &&
					varDecl->annotation().type->isValueType()
				)
					m_touchedVariable[&n] = varDecl;
		}
		return true;
	};
	auto edgeFun = [&](ASTNode const& _parent, ASTNode const& _child)
	{
		if (m_touchedVariable.count(&_child) || m_children.count(&_child))
			m_children[&_parent].push_back(&_child);
	};

	ASTReduce reducer(nodeFun, edgeFun);
	_node.accept(reducer);
}

vector<Declaration const*> VariableUsage::touchedVariables(ASTNode const& _node) const
{
	if (!m_children.count(&_node) && !m_touchedVariable.count(&_node))
		return {};

	set<Declaration const*> touched;
	vector<ASTNode const*> toVisit;
	toVisit.push_back(&_node);

	while (!toVisit.empty())
	{
		ASTNode const* n = toVisit.back();
		toVisit.pop_back();
		if (m_children.count(n))
		{
			polAssert(!m_touchedVariable.count(n), "");
			toVisit += m_children.at(n);
		}
		else
		{
			polAssert(m_touchedVariable.count(n), "");
			touched.insert(m_touchedVariable.at(n));
		}
	}

	return {touched.begin(), touched.end()};
}
