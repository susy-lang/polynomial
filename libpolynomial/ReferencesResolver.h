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
 * @date 2014
 * Component that resolves type names to types and annotates the AST accordingly.
 */

#pragma once

#include <map>
#include <list>
#include <boost/noncopyable.hpp>
#include <libpolynomial/ASTVisitor.h>
#include <libpolynomial/ASTAnnotations.h>

namespace dev
{
namespace polynomial
{

class NameAndTypeResolver;

/**
 * Resolves references to declarations (of variables and types) and also establishes the link
 * between a return statement and the return parameter list.
 */
class ReferencesResolver: private ASTConstVisitor
{
public:
	ReferencesResolver(
		ASTNode& _root,
		NameAndTypeResolver& _resolver,
		ContractDefinition const* _currentContract,
		ParameterList const* _returnParameters,
		bool _resolveInsideCode = false
	);

private:
	virtual bool visit(Block const&) override { return m_resolveInsideCode; }
	virtual bool visit(Identifier const& _identifier) override;
	virtual bool visit(UserDefinedTypeName const& _typeName) override;
	virtual bool visit(Return const& _return) override;
	virtual void endVisit(VariableDeclaration const& _variable) override;

	TypePointer typeFor(TypeName const& _typeName);

	NameAndTypeResolver& m_resolver;
	ContractDefinition const* m_currentContract;
	ParameterList const* m_returnParameters;
	bool m_resolveInsideCode;
};

}
}
