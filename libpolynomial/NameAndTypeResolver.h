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
 * Parser part that determines the declarations corresponding to names and the types of expressions.
 */

#pragma once

#include <map>
#include <list>
#include <boost/noncopyable.hpp>
#include <libpolynomial/DeclarationContainer.h>
#include <libpolynomial/ReferencesResolver.h>
#include <libpolynomial/ASTVisitor.h>
#include <libpolynomial/ASTAnnotations.h>

namespace dev
{
namespace polynomial
{

/**
 * Resolves name references, typenames and sets the (explicitly given) types for all variable
 * declarations.
 */
class NameAndTypeResolver: private boost::noncopyable
{
public:
	NameAndTypeResolver(std::vector<Declaration const*> const& _globals);
	/// Registers all declarations found in the source unit.
	void registerDeclarations(SourceUnit& _sourceUnit);
	/// Resolves all names and types referenced from the given contract.
	void resolveNamesAndTypes(ContractDefinition& _contract);
	/// Updates the given global declaration (used for "this"). Not to be used with declarations
	/// that create their own scope.
	void updateDeclaration(Declaration const& _declaration);

	/// Resolves the given @a _name inside the scope @a _scope. If @a _scope is omitted,
	/// the global scope is used (i.e. the one containing only the contract).
	/// @returns a pointer to the declaration on success or nullptr on failure.
	std::vector<Declaration const*> resolveName(ASTString const& _name, Declaration const* _scope = nullptr) const;

	/// Resolves a name in the "current" scope. Should only be called during the initial
	/// repolving phase.
	std::vector<Declaration const*> nameFromCurrentScope(ASTString const& _name, bool _recursive = true) const;

	/// Resolves a path starting from the "current" scope. Should only be called during the initial
	/// repolving phase.
	/// @note Returns a null pointer if any component in the path was not unique or not found.
	Declaration const* pathFromCurrentScope(std::vector<ASTString> const& _path, bool _recursive = true) const;

	/// returns the vector of declarations without repetitions
	static std::vector<Declaration const*> cleanedDeclarations(
		Identifier const& _identifier,
		std::vector<Declaration const*> const& _declarations
	);

private:
	void reset();

	/// Imports all members declared directly in the given contract (i.e. does not import inherited members)
	/// into the current scope if they are not present already.
	void importInheritedScope(ContractDefinition const& _base);

	/// Computes "C3-Linearization" of base contracts and stores it inside the contract.
	void linearizeBaseContracts(ContractDefinition& _contract) const;
	/// Computes the C3-merge of the given list of lists of bases.
	/// @returns the linearized vector or an empty vector if linearization is not possible.
	template <class _T>
	static std::vector<_T const*> cThreeMerge(std::list<std::list<_T const*>>& _toMerge);

	/// Maps nodes declaring a scope to scopes, i.e. ContractDefinition and FunctionDeclaration,
	/// where nullptr denotes the global scope. Note that structs are not scope since they do
	/// not contain code.
	std::map<ASTNode const*, DeclarationContainer> m_scopes;

	DeclarationContainer* m_currentScope = nullptr;
};

/**
 * Traverses the given AST upon construction and fills _scopes with all declarations inside the
 * AST.
 */
class DeclarationRegistrationHelper: private ASTVisitor
{
public:
	DeclarationRegistrationHelper(std::map<ASTNode const*, DeclarationContainer>& _scopes, ASTNode& _astRoot);

private:
	bool visit(ContractDefinition& _contract) override;
	void endVisit(ContractDefinition& _contract) override;
	bool visit(StructDefinition& _struct) override;
	void endVisit(StructDefinition& _struct) override;
	bool visit(EnumDefinition& _enum) override;
	void endVisit(EnumDefinition& _enum) override;
	bool visit(EnumValue& _value) override;
	bool visit(FunctionDefinition& _function) override;
	void endVisit(FunctionDefinition& _function) override;
	bool visit(ModifierDefinition& _modifier) override;
	void endVisit(ModifierDefinition& _modifier) override;
	void endVisit(VariableDeclarationStatement& _variableDeclarationStatement) override;
	bool visit(VariableDeclaration& _declaration) override;
	bool visit(EventDefinition& _event) override;
	void endVisit(EventDefinition& _event) override;

	void enterNewSubScope(Declaration const& _declaration);
	void closeCurrentScope();
	void registerDeclaration(Declaration& _declaration, bool _opensScope);

	/// @returns the canonical name of the current scope.
	std::string currentCanonicalName() const;

	std::map<ASTNode const*, DeclarationContainer>& m_scopes;
	Declaration const* m_currentScope;
	VariableScope* m_currentFunction;
};

}
}
