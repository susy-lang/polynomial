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
 * Utilities for the polynomial compiler.
 */

#include <utility>
#include <numeric>
#include <libpolynomial/AST.h>
#include <libpolynomial/Compiler.h>

using namespace std;

namespace dev
{
namespace polynomial
{

void CompilerContext::addMagicGlobal(MagicVariableDeclaration const& _declaration)
{
	m_magicGlobals.insert(&_declaration);
}

void CompilerContext::addStateVariable(
	VariableDeclaration const& _declaration,
	u256 const& _storageOffset,
	unsigned _byteOffset
)
{
	m_stateVariables[&_declaration] = make_pair(_storageOffset, _byteOffset);
}

void CompilerContext::startFunction(Declaration const& _function)
{
	m_functionsWithCode.insert(&_function);
	*this << functionEntryLabel(_function);
}

void CompilerContext::addVariable(VariableDeclaration const& _declaration,
								  unsigned _offsetToCurrent)
{
	polAssert(m_asm.deposit() >= 0 && unsigned(m_asm.deposit()) >= _offsetToCurrent, "");
	m_localVariables[&_declaration] = unsigned(m_asm.deposit()) - _offsetToCurrent;
}

void CompilerContext::removeVariable(VariableDeclaration const& _declaration)
{
	polAssert(!!m_localVariables.count(&_declaration), "");
	m_localVariables.erase(&_declaration);
}

sof::Assembly const& CompilerContext::compiledContract(const ContractDefinition& _contract) const
{
	auto ret = m_compiledContracts.find(&_contract);
	polAssert(ret != m_compiledContracts.end(), "Compiled contract not found.");
	return *ret->second;
}

bool CompilerContext::isLocalVariable(Declaration const* _declaration) const
{
	return !!m_localVariables.count(_declaration);
}

sof::AssemblyItem CompilerContext::functionEntryLabel(Declaration const& _declaration)
{
	auto res = m_functionEntryLabels.find(&_declaration);
	if (res == m_functionEntryLabels.end())
	{
		sof::AssemblyItem tag(m_asm.newTag());
		m_functionEntryLabels.insert(make_pair(&_declaration, tag));
		return tag.tag();
	}
	else
		return res->second.tag();
}

sof::AssemblyItem CompilerContext::functionEntryLabelIfExists(Declaration const& _declaration) const
{
	auto res = m_functionEntryLabels.find(&_declaration);
	return res == m_functionEntryLabels.end() ? sof::AssemblyItem(sof::UndefinedItem) : res->second.tag();
}

sof::AssemblyItem CompilerContext::virtualFunctionEntryLabel(FunctionDefinition const& _function)
{
	polAssert(!m_inheritanceHierarchy.empty(), "No inheritance hierarchy set.");
	return virtualFunctionEntryLabel(_function, m_inheritanceHierarchy.begin());
}

sof::AssemblyItem CompilerContext::superFunctionEntryLabel(FunctionDefinition const& _function, ContractDefinition const& _base)
{
	polAssert(!m_inheritanceHierarchy.empty(), "No inheritance hierarchy set.");
	return virtualFunctionEntryLabel(_function, superContract(_base));
}

FunctionDefinition const* CompilerContext::nextConstructor(ContractDefinition const& _contract) const
{
	vector<ContractDefinition const*>::const_iterator it = superContract(_contract);
	for (; it != m_inheritanceHierarchy.end(); ++it)
		if ((*it)->constructor())
			return (*it)->constructor();

	return nullptr;
}

set<Declaration const*> CompilerContext::functionsWithoutCode()
{
	set<Declaration const*> functions;
	for (auto const& it: m_functionEntryLabels)
		if (m_functionsWithCode.count(it.first) == 0)
			functions.insert(it.first);
	return functions;
}

ModifierDefinition const& CompilerContext::functionModifier(string const& _name) const
{
	polAssert(!m_inheritanceHierarchy.empty(), "No inheritance hierarchy set.");
	for (ContractDefinition const* contract: m_inheritanceHierarchy)
		for (ASTPointer<ModifierDefinition> const& modifier: contract->functionModifiers())
			if (modifier->name() == _name)
				return *modifier.get();
	BOOST_THROW_EXCEPTION(InternalCompilerError()
						  << errinfo_comment("Function modifier " + _name + " not found."));
}

unsigned CompilerContext::baseStackOffsetOfVariable(Declaration const& _declaration) const
{
	auto res = m_localVariables.find(&_declaration);
	polAssert(res != m_localVariables.end(), "Variable not found on stack.");
	return res->second;
}

unsigned CompilerContext::baseToCurrentStackOffset(unsigned _baseOffset) const
{
	return m_asm.deposit() - _baseOffset - 1;
}

unsigned CompilerContext::currentToBaseStackOffset(unsigned _offset) const
{
	return m_asm.deposit() - _offset - 1;
}

pair<u256, unsigned> CompilerContext::storageLocationOfVariable(const Declaration& _declaration) const
{
	auto it = m_stateVariables.find(&_declaration);
	polAssert(it != m_stateVariables.end(), "Variable not found in storage.");
	return it->second;
}

CompilerContext& CompilerContext::appendJump(sof::AssemblyItem::JumpType _jumpType)
{
	sof::AssemblyItem item(sof::Instruction::JUMP);
	item.setJumpType(_jumpType);
	return *this << item;
}

void CompilerContext::resetVisitedNodes(ASTNode const* _node)
{
	stack<ASTNode const*> newStack;
	newStack.push(_node);
	std::swap(m_visitedNodes, newStack);
	updateSourceLocation();
}

sof::AssemblyItem CompilerContext::virtualFunctionEntryLabel(
	FunctionDefinition const& _function,
	vector<ContractDefinition const*>::const_iterator _searchStart
)
{
	string name = _function.name();
	FunctionType functionType(_function);
	auto it = _searchStart;
	for (; it != m_inheritanceHierarchy.end(); ++it)
		for (ASTPointer<FunctionDefinition> const& function: (*it)->definedFunctions())
			if (
				function->name() == name &&
				!function->isConstructor() &&
				FunctionType(*function).hasEqualArgumentTypes(functionType)
			)
				return functionEntryLabel(*function);
	polAssert(false, "Super function " + name + " not found.");
	return m_asm.newTag(); // not reached
}

vector<ContractDefinition const*>::const_iterator CompilerContext::superContract(ContractDefinition const& _contract) const
{
	polAssert(!m_inheritanceHierarchy.empty(), "No inheritance hierarchy set.");
	auto it = find(m_inheritanceHierarchy.begin(), m_inheritanceHierarchy.end(), &_contract);
	polAssert(it != m_inheritanceHierarchy.end(), "Base not found in inheritance hierarchy.");
	return ++it;
}

void CompilerContext::updateSourceLocation()
{
	m_asm.setSourceLocation(m_visitedNodes.empty() ? SourceLocation() : m_visitedNodes.top()->location());
}

}
}
