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
 * Polynomial abstract syntax tree.
 */

#include <algorithm>
#include <functional>
#include <libpolynomial/interface/Utils.h>
#include <libpolynomial/ast/AST.h>
#include <libpolynomial/ast/ASTVisitor.h>
#include <libpolynomial/interface/Exceptions.h>
#include <libpolynomial/ast/AST_accept.h>

#include <libdevcore/SHA3.h>

using namespace std;
using namespace dev;
using namespace dev::polynomial;

ASTNode::ASTNode(SourceLocation const& _location):
	m_location(_location)
{
}

ASTNode::~ASTNode()
{
	delete m_annotation;
}

ASTAnnotation& ASTNode::annotation() const
{
	if (!m_annotation)
		m_annotation = new ASTAnnotation();
	return *m_annotation;
}

Error ASTNode::createTypeError(string const& _description) const
{
	return Error(Error::Type::TypeError) << errinfo_sourceLocation(location()) << errinfo_comment(_description);
}

SourceUnitAnnotation& SourceUnit::annotation() const
{
	if (!m_annotation)
		m_annotation = new SourceUnitAnnotation();
	return static_cast<SourceUnitAnnotation&>(*m_annotation);
}

ImportAnnotation& ImportDirective::annotation() const
{
	if (!m_annotation)
		m_annotation = new ImportAnnotation();
	return static_cast<ImportAnnotation&>(*m_annotation);
}

TypePointer ImportDirective::type() const
{
	polAssert(!!annotation().sourceUnit, "");
	return make_shared<ModuleType>(*annotation().sourceUnit);
}

map<FixedHash<4>, FunctionTypePointer> ContractDefinition::interfaceFunctions() const
{
	auto exportedFunctionList = interfaceFunctionList();

	map<FixedHash<4>, FunctionTypePointer> exportedFunctions;
	for (auto const& it: exportedFunctionList)
		exportedFunctions.insert(it);

	polAssert(
		exportedFunctionList.size() == exportedFunctions.size(),
		"Hash collision at Function Definition Hash calculation"
	);

	return exportedFunctions;
}

FunctionDefinition const* ContractDefinition::constructor() const
{
	for (FunctionDefinition const* f: definedFunctions())
		if (f->isConstructor())
			return f;
	return nullptr;
}

FunctionDefinition const* ContractDefinition::fallbackFunction() const
{
	for (ContractDefinition const* contract: annotation().linearizedBaseContracts)
		for (FunctionDefinition const* f: contract->definedFunctions())
			if (f->name().empty())
				return f;
	return nullptr;
}

vector<EventDefinition const*> const& ContractDefinition::interfaceEvents() const
{
	if (!m_interfaceEvents)
	{
		set<string> eventsSeen;
		m_interfaceEvents.reset(new vector<EventDefinition const*>());
		for (ContractDefinition const* contract: annotation().linearizedBaseContracts)
			for (EventDefinition const* e: contract->events())
				if (eventsSeen.count(e->name()) == 0)
				{
					eventsSeen.insert(e->name());
					m_interfaceEvents->push_back(e);
				}
	}
	return *m_interfaceEvents;
}

vector<pair<FixedHash<4>, FunctionTypePointer>> const& ContractDefinition::interfaceFunctionList() const
{
	if (!m_interfaceFunctionList)
	{
		set<string> functionsSeen;
		set<string> signaturesSeen;
		m_interfaceFunctionList.reset(new vector<pair<FixedHash<4>, FunctionTypePointer>>());
		for (ContractDefinition const* contract: annotation().linearizedBaseContracts)
		{
			vector<FunctionTypePointer> functions;
			for (FunctionDefinition const* f: contract->definedFunctions())
				if (f->isPartOfExternalInterface())
					functions.push_back(make_shared<FunctionType>(*f, false));
			for (VariableDeclaration const* v: contract->stateVariables())
				if (v->isPartOfExternalInterface())
					functions.push_back(make_shared<FunctionType>(*v));
			for (FunctionTypePointer const& fun: functions)
			{
				if (!fun->interfaceFunctionType())
					// Fails hopefully because we already registered the error
					continue;
				string functionSignature = fun->externalSignature();
				if (signaturesSeen.count(functionSignature) == 0)
				{
					signaturesSeen.insert(functionSignature);
					FixedHash<4> hash(dev::sha3(functionSignature));
					m_interfaceFunctionList->push_back(make_pair(hash, fun));
				}
			}
		}
	}
	return *m_interfaceFunctionList;
}

string const& ContractDefinition::devDocumentation() const
{
	return m_devDocumentation;
}

string const& ContractDefinition::userDocumentation() const
{
	return m_userDocumentation;
}

void ContractDefinition::setDevDocumentation(string const& _devDocumentation)
{
	m_devDocumentation = _devDocumentation;
}

void ContractDefinition::setUserDocumentation(string const& _userDocumentation)
{
	m_userDocumentation = _userDocumentation;
}


vector<Declaration const*> const& ContractDefinition::inheritableMembers() const
{
	if (!m_inheritableMembers)
	{
		set<string> memberSeen;
		m_inheritableMembers.reset(new vector<Declaration const*>());
		auto addInheritableMember = [&](Declaration const* _decl)
		{
			if (memberSeen.count(_decl->name()) == 0 && _decl->isVisibleInDerivedContracts())
			{
				memberSeen.insert(_decl->name());
				m_inheritableMembers->push_back(_decl);
			}
		};

		for (FunctionDefinition const* f: definedFunctions())
			addInheritableMember(f);

		for (VariableDeclaration const* v: stateVariables())
			addInheritableMember(v);

		for (StructDefinition const* s: definedStructs())
			addInheritableMember(s);
	}
	return *m_inheritableMembers;
}

TypePointer ContractDefinition::type() const
{
	return make_shared<TypeType>(make_shared<ContractType>(*this));
}

ContractDefinitionAnnotation& ContractDefinition::annotation() const
{
	if (!m_annotation)
		m_annotation = new ContractDefinitionAnnotation();
	return static_cast<ContractDefinitionAnnotation&>(*m_annotation);
}

TypeNameAnnotation& TypeName::annotation() const
{
	if (!m_annotation)
		m_annotation = new TypeNameAnnotation();
	return static_cast<TypeNameAnnotation&>(*m_annotation);
}

TypePointer StructDefinition::type() const
{
	return make_shared<TypeType>(make_shared<StructType>(*this));
}

TypeDeclarationAnnotation& StructDefinition::annotation() const
{
	if (!m_annotation)
		m_annotation = new TypeDeclarationAnnotation();
	return static_cast<TypeDeclarationAnnotation&>(*m_annotation);
}

TypePointer EnumValue::type() const
{
	auto parentDef = dynamic_cast<EnumDefinition const*>(scope());
	polAssert(parentDef, "Enclosing Scope of EnumValue was not set");
	return make_shared<EnumType>(*parentDef);
}

TypePointer EnumDefinition::type() const
{
	return make_shared<TypeType>(make_shared<EnumType>(*this));
}

TypeDeclarationAnnotation& EnumDefinition::annotation() const
{
	if (!m_annotation)
		m_annotation = new TypeDeclarationAnnotation();
	return static_cast<TypeDeclarationAnnotation&>(*m_annotation);
}

TypePointer FunctionDefinition::type() const
{
	return make_shared<FunctionType>(*this);
}

string FunctionDefinition::externalSignature() const
{
	return FunctionType(*this).externalSignature();
}

FunctionDefinitionAnnotation& FunctionDefinition::annotation() const
{
	if (!m_annotation)
		m_annotation = new FunctionDefinitionAnnotation();
	return static_cast<FunctionDefinitionAnnotation&>(*m_annotation);
}

TypePointer ModifierDefinition::type() const
{
	return make_shared<ModifierType>(*this);
}

ModifierDefinitionAnnotation& ModifierDefinition::annotation() const
{
	if (!m_annotation)
		m_annotation = new ModifierDefinitionAnnotation();
	return static_cast<ModifierDefinitionAnnotation&>(*m_annotation);
}

TypePointer EventDefinition::type() const
{
	return make_shared<FunctionType>(*this);
}

EventDefinitionAnnotation& EventDefinition::annotation() const
{
	if (!m_annotation)
		m_annotation = new EventDefinitionAnnotation();
	return static_cast<EventDefinitionAnnotation&>(*m_annotation);
}

UserDefinedTypeNameAnnotation& UserDefinedTypeName::annotation() const
{
	if (!m_annotation)
		m_annotation = new UserDefinedTypeNameAnnotation();
	return static_cast<UserDefinedTypeNameAnnotation&>(*m_annotation);
}

bool VariableDeclaration::isLValue() const
{
	// External function parameters and constant declared variables are Read-Only
	return !isExternalCallableParameter() && !m_isConstant;
}

bool VariableDeclaration::isCallableParameter() const
{
	auto const* callable = dynamic_cast<CallableDeclaration const*>(scope());
	if (!callable)
		return false;
	for (auto const& variable: callable->parameters())
		if (variable.get() == this)
			return true;
	if (callable->returnParameterList())
		for (auto const& variable: callable->returnParameterList()->parameters())
			if (variable.get() == this)
				return true;
	return false;
}

bool VariableDeclaration::isExternalCallableParameter() const
{
	auto const* callable = dynamic_cast<CallableDeclaration const*>(scope());
	if (!callable || callable->visibility() != Declaration::Visibility::External)
		return false;
	for (auto const& variable: callable->parameters())
		if (variable.get() == this)
			return true;
	return false;
}

bool VariableDeclaration::canHaveAutoType() const
{
	auto const* callable = dynamic_cast<CallableDeclaration const*>(scope());
	return (!!callable && !isCallableParameter());
}

TypePointer VariableDeclaration::type() const
{
	return annotation().type;
}

VariableDeclarationAnnotation& VariableDeclaration::annotation() const
{
	if (!m_annotation)
		m_annotation = new VariableDeclarationAnnotation();
	return static_cast<VariableDeclarationAnnotation&>(*m_annotation);
}

StatementAnnotation& Statement::annotation() const
{
	if (!m_annotation)
		m_annotation = new StatementAnnotation();
	return static_cast<StatementAnnotation&>(*m_annotation);
}

InlineAssemblyAnnotation& InlineAssembly::annotation() const
{
	if (!m_annotation)
		m_annotation = new InlineAssemblyAnnotation();
	return static_cast<InlineAssemblyAnnotation&>(*m_annotation);
}

ReturnAnnotation& Return::annotation() const
{
	if (!m_annotation)
		m_annotation = new ReturnAnnotation();
	return static_cast<ReturnAnnotation&>(*m_annotation);
}

VariableDeclarationStatementAnnotation& VariableDeclarationStatement::annotation() const
{
	if (!m_annotation)
		m_annotation = new VariableDeclarationStatementAnnotation();
	return static_cast<VariableDeclarationStatementAnnotation&>(*m_annotation);
}

ExpressionAnnotation& Expression::annotation() const
{
	if (!m_annotation)
		m_annotation = new ExpressionAnnotation();
	return static_cast<ExpressionAnnotation&>(*m_annotation);
}

MemberAccessAnnotation& MemberAccess::annotation() const
{
	if (!m_annotation)
		m_annotation = new MemberAccessAnnotation();
	return static_cast<MemberAccessAnnotation&>(*m_annotation);
}

BinaryOperationAnnotation& BinaryOperation::annotation() const
{
	if (!m_annotation)
		m_annotation = new BinaryOperationAnnotation();
	return static_cast<BinaryOperationAnnotation&>(*m_annotation);
}

FunctionCallAnnotation& FunctionCall::annotation() const
{
	if (!m_annotation)
		m_annotation = new FunctionCallAnnotation();
	return static_cast<FunctionCallAnnotation&>(*m_annotation);
}

IdentifierAnnotation& Identifier::annotation() const
{
	if (!m_annotation)
		m_annotation = new IdentifierAnnotation();
	return static_cast<IdentifierAnnotation&>(*m_annotation);
}
