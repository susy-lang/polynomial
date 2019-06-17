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
/**
 * Common code generator for translating Yul / inline assembly to SVM and SVM1.5.
 */

#include <libyul/backends/svm/SVMCodeTransform.h>

#include <libyul/optimiser/NameCollector.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmData.h>

#include <liblangutil/Exceptions.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace dev;
using namespace yul;
using namespace dev::polynomial;

void VariableReferenceCounter::operator()(Identifier const& _identifier)
{
	increaseRefIfFound(_identifier.name);
}

void VariableReferenceCounter::operator()(FunctionDefinition const& _function)
{
	Scope* originalScope = m_scope;

	polAssert(m_info.virtualBlocks.at(&_function), "");
	m_scope = m_info.scopes.at(m_info.virtualBlocks.at(&_function).get()).get();
	polAssert(m_scope, "Variable scope does not exist.");

	for (auto const& v: _function.returnVariables)
		increaseRefIfFound(v.name);

	VariableReferenceCounter{m_context, m_info}(_function.body);

	m_scope = originalScope;
}

void VariableReferenceCounter::operator()(ForLoop const& _forLoop)
{
	Scope* originalScope = m_scope;
	// Special scoping rules.
	m_scope = m_info.scopes.at(&_forLoop.pre).get();

	walkVector(_forLoop.pre.statements);
	visit(*_forLoop.condition);
	(*this)(_forLoop.body);
	(*this)(_forLoop.post);

	m_scope = originalScope;
}


void VariableReferenceCounter::operator()(Block const& _block)
{
	Scope* originalScope = m_scope;
	m_scope = m_info.scopes.at(&_block).get();

	ASTWalker::operator()(_block);

	m_scope = originalScope;
}

void VariableReferenceCounter::increaseRefIfFound(YulString _variableName)
{
	m_scope->lookup(_variableName, Scope::Visitor(
		[=](Scope::Variable const& _var)
		{
			++m_context.variableReferences[&_var];
		},
		[=](Scope::Label const&) { },
		[=](Scope::Function const&) { }
	));
}


CodeTransform::CodeTransform(
	AbstractAssembly& _assembly,
	AsmAnalysisInfo& _analysisInfo,
	Block const& _block,
	bool _allowStackOpt,
	SVMDialect const& _dialect,
	bool _svm15,
	ExternalIdentifierAccess const& _identifierAccess,
	bool _useNamedLabelsForFunctions,
	int _stackAdjustment,
	shared_ptr<Context> _context
):
	m_assembly(_assembly),
	m_info(_analysisInfo),
	m_dialect(_dialect),
	m_allowStackOpt(_allowStackOpt),
	m_svm15(_svm15),
	m_useNamedLabelsForFunctions(_useNamedLabelsForFunctions),
	m_identifierAccess(_identifierAccess),
	m_stackAdjustment(_stackAdjustment),
	m_context(_context)
{
	if (!m_context)
	{
		// initialize
		m_context = make_shared<Context>();
		if (m_allowStackOpt)
			VariableReferenceCounter{*m_context, m_info}(_block);
	}
}

void CodeTransform::decreaseReference(YulString, Scope::Variable const& _var)
{
	if (!m_allowStackOpt)
		return;

	unsigned& ref = m_context->variableReferences.at(&_var);
	polAssert(ref >= 1, "");
	--ref;
	if (ref == 0)
		m_variablesScheduledForDeletion.insert(&_var);
}

bool CodeTransform::unreferenced(Scope::Variable const& _var) const
{
	return !m_context->variableReferences.count(&_var) || m_context->variableReferences[&_var] == 0;
}

void CodeTransform::freeUnusedVariables()
{
	if (!m_allowStackOpt)
		return;

	for (auto const& identifier: m_scope->identifiers)
		if (identifier.second.type() == typeid(Scope::Variable))
		{
			Scope::Variable const& var = boost::get<Scope::Variable>(identifier.second);
			if (m_variablesScheduledForDeletion.count(&var))
				deleteVariable(var);
		}

	while (m_unusedStackSlots.count(m_assembly.stackHeight() - 1))
	{
		polAssert(m_unusedStackSlots.erase(m_assembly.stackHeight() - 1), "");
		m_assembly.appendInstruction(polynomial::Instruction::POP);
		--m_stackAdjustment;
	}
}

void CodeTransform::deleteVariable(Scope::Variable const& _var)
{
	polAssert(m_allowStackOpt, "");
	polAssert(m_context->variableStackHeights.count(&_var) > 0, "");
	m_unusedStackSlots.insert(m_context->variableStackHeights[&_var]);
	m_context->variableStackHeights.erase(&_var);
	m_context->variableReferences.erase(&_var);
	m_variablesScheduledForDeletion.erase(&_var);
}

void CodeTransform::operator()(VariableDeclaration const& _varDecl)
{
	polAssert(m_scope, "");

	int const numVariables = _varDecl.variables.size();
	int height = m_assembly.stackHeight();
	if (_varDecl.value)
	{
		boost::apply_visitor(*this, *_varDecl.value);
		expectDeposit(numVariables, height);
	}
	else
	{
		int variablesLeft = numVariables;
		while (variablesLeft--)
			m_assembly.appendConstant(u256(0));
	}

	bool atTopOfStack = true;
	for (int varIndex = numVariables - 1; varIndex >= 0; --varIndex)
	{
		auto& var = boost::get<Scope::Variable>(m_scope->identifiers.at(_varDecl.variables[varIndex].name));
		m_context->variableStackHeights[&var] = height + varIndex;
		if (!m_allowStackOpt)
			continue;

		if (unreferenced(var))
		{
			if (atTopOfStack)
			{
				m_context->variableStackHeights.erase(&var);
				m_assembly.setSourceLocation(_varDecl.location);
				m_assembly.appendInstruction(polynomial::Instruction::POP);
				--m_stackAdjustment;
			}
			else
				m_variablesScheduledForDeletion.insert(&var);
		}
		else if (m_unusedStackSlots.empty())
			atTopOfStack = false;
		else
		{
			int slot = *m_unusedStackSlots.begin();
			m_unusedStackSlots.erase(m_unusedStackSlots.begin());
			m_context->variableStackHeights[&var] = slot;
			m_assembly.setSourceLocation(_varDecl.location);
			if (int heightDiff = variableHeightDiff(var, true))
				m_assembly.appendInstruction(polynomial::swapInstruction(heightDiff - 1));
			m_assembly.appendInstruction(polynomial::Instruction::POP);
			--m_stackAdjustment;
		}
	}
	checkStackHeight(&_varDecl);
}

void CodeTransform::operator()(Assignment const& _assignment)
{
	int height = m_assembly.stackHeight();
	boost::apply_visitor(*this, *_assignment.value);
	expectDeposit(_assignment.variableNames.size(), height);

	m_assembly.setSourceLocation(_assignment.location);
	generateMultiAssignment(_assignment.variableNames);
	checkStackHeight(&_assignment);
}

void CodeTransform::operator()(StackAssignment const& _assignment)
{
	polAssert(!m_allowStackOpt, "");
	m_assembly.setSourceLocation(_assignment.location);
	generateAssignment(_assignment.variableName);
	checkStackHeight(&_assignment);
}

void CodeTransform::operator()(ExpressionStatement const& _statement)
{
	m_assembly.setSourceLocation(_statement.location);
	boost::apply_visitor(*this, _statement.expression);
	checkStackHeight(&_statement);
}

void CodeTransform::operator()(Label const& _label)
{
	polAssert(!m_allowStackOpt, "");
	m_assembly.setSourceLocation(_label.location);
	polAssert(m_scope, "");
	polAssert(m_scope->identifiers.count(_label.name), "");
	Scope::Label& label = boost::get<Scope::Label>(m_scope->identifiers.at(_label.name));
	m_assembly.appendLabel(labelID(label));
	checkStackHeight(&_label);
}

void CodeTransform::operator()(FunctionCall const& _call)
{
	polAssert(m_scope, "");

	if (BuiltinFunctionForSVM const* builtin = m_dialect.builtin(_call.functionName.name))
	{
		builtin->generateCode(_call, m_assembly, [&]() {
			for (auto const& arg: _call.arguments | boost::adaptors::reversed)
				visitExpression(arg);
			m_assembly.setSourceLocation(_call.location);
		});
	}
	else
	{
		m_assembly.setSourceLocation(_call.location);
		SVMAssembly::LabelID returnLabel(-1); // only used for svm 1.0
		if (!m_svm15)
		{
			returnLabel = m_assembly.newLabelId();
			m_assembly.appendLabelReference(returnLabel);
			m_stackAdjustment++;
		}

		Scope::Function* function = nullptr;
		polAssert(m_scope->lookup(_call.functionName.name, Scope::NonconstVisitor(
			[=](Scope::Variable&) { polAssert(false, "Expected function name."); },
			[=](Scope::Label&) { polAssert(false, "Expected function name."); },
			[&](Scope::Function& _function) { function = &_function; }
		)), "Function name not found.");
		polAssert(function, "");
		polAssert(function->arguments.size() == _call.arguments.size(), "");
		for (auto const& arg: _call.arguments | boost::adaptors::reversed)
			visitExpression(arg);
		m_assembly.setSourceLocation(_call.location);
		if (m_svm15)
			m_assembly.appendJumpsub(functionEntryID(_call.functionName.name, *function), function->arguments.size(), function->returns.size());
		else
		{
			m_assembly.appendJumpTo(functionEntryID(_call.functionName.name, *function), function->returns.size() - function->arguments.size() - 1);
			m_assembly.appendLabel(returnLabel);
			m_stackAdjustment--;
		}
		checkStackHeight(&_call);
	}
}

void CodeTransform::operator()(FunctionalInstruction const& _instruction)
{
	if (m_svm15 && (
		_instruction.instruction == polynomial::Instruction::JUMP ||
		_instruction.instruction == polynomial::Instruction::JUMPI
	))
	{
		bool const isJumpI = _instruction.instruction == polynomial::Instruction::JUMPI;
		if (isJumpI)
		{
			polAssert(_instruction.arguments.size() == 2, "");
			visitExpression(_instruction.arguments.at(1));
		}
		else
		{
			polAssert(_instruction.arguments.size() == 1, "");
		}
		m_assembly.setSourceLocation(_instruction.location);
		auto label = labelFromIdentifier(boost::get<Identifier>(_instruction.arguments.at(0)));
		if (isJumpI)
			m_assembly.appendJumpToIf(label);
		else
			m_assembly.appendJumpTo(label);
	}
	else
	{
		for (auto const& arg: _instruction.arguments | boost::adaptors::reversed)
			visitExpression(arg);
		m_assembly.setSourceLocation(_instruction.location);
		m_assembly.appendInstruction(_instruction.instruction);
	}
	checkStackHeight(&_instruction);
}

void CodeTransform::operator()(Identifier const& _identifier)
{
	m_assembly.setSourceLocation(_identifier.location);
	// First search internals, then externals.
	polAssert(m_scope, "");
	if (m_scope->lookup(_identifier.name, Scope::NonconstVisitor(
		[=](Scope::Variable& _var)
		{
			// TODO: opportunity for optimization: Do not DUP if this is the last reference
			// to the top most element of the stack
			if (int heightDiff = variableHeightDiff(_var, false))
				m_assembly.appendInstruction(polynomial::dupInstruction(heightDiff));
			else
				// Store something to balance the stack
				m_assembly.appendConstant(u256(0));
			decreaseReference(_identifier.name, _var);
		},
		[=](Scope::Label& _label)
		{
			m_assembly.appendLabelReference(labelID(_label));
		},
		[=](Scope::Function&)
		{
			polAssert(false, "Function not removed during desugaring.");
		}
	)))
	{
		return;
	}
	polAssert(
		m_identifierAccess.generateCode,
		"Identifier not found and no external access available."
	);
	m_identifierAccess.generateCode(_identifier, IdentifierContext::RValue, m_assembly);
	checkStackHeight(&_identifier);
}

void CodeTransform::operator()(Literal const& _literal)
{
	m_assembly.setSourceLocation(_literal.location);
	if (_literal.kind == LiteralKind::Number)
		m_assembly.appendConstant(u256(_literal.value.str()));
	else if (_literal.kind == LiteralKind::Boolean)
	{
		if (_literal.value == "true"_yulstring)
			m_assembly.appendConstant(u256(1));
		else
			m_assembly.appendConstant(u256(0));
	}
	else
	{
		polAssert(_literal.value.str().size() <= 32, "");
		m_assembly.appendConstant(u256(h256(_literal.value.str(), h256::FromBinary, h256::AlignLeft)));
	}
	checkStackHeight(&_literal);
}

void CodeTransform::operator()(yul::Instruction const& _instruction)
{
	polAssert(!m_allowStackOpt, "");
	polAssert(!m_svm15 || _instruction.instruction != polynomial::Instruction::JUMP, "Bare JUMP instruction used for SVM1.5");
	polAssert(!m_svm15 || _instruction.instruction != polynomial::Instruction::JUMPI, "Bare JUMPI instruction used for SVM1.5");
	m_assembly.setSourceLocation(_instruction.location);
	m_assembly.appendInstruction(_instruction.instruction);
	checkStackHeight(&_instruction);
}

void CodeTransform::operator()(If const& _if)
{
	visitExpression(*_if.condition);
	m_assembly.setSourceLocation(_if.location);
	m_assembly.appendInstruction(polynomial::Instruction::ISZERO);
	AbstractAssembly::LabelID end = m_assembly.newLabelId();
	m_assembly.appendJumpToIf(end);
	(*this)(_if.body);
	m_assembly.setSourceLocation(_if.location);
	m_assembly.appendLabel(end);
	checkStackHeight(&_if);
}

void CodeTransform::operator()(Switch const& _switch)
{
	//@TODO use JUMPV in SVM1.5?

	visitExpression(*_switch.expression);
	int expressionHeight = m_assembly.stackHeight();
	map<Case const*, AbstractAssembly::LabelID> caseBodies;
	AbstractAssembly::LabelID end = m_assembly.newLabelId();
	for (Case const& c: _switch.cases)
	{
		if (c.value)
		{
			(*this)(*c.value);
			m_assembly.setSourceLocation(c.location);
			AbstractAssembly::LabelID bodyLabel = m_assembly.newLabelId();
			caseBodies[&c] = bodyLabel;
			polAssert(m_assembly.stackHeight() == expressionHeight + 1, "");
			m_assembly.appendInstruction(polynomial::dupInstruction(2));
			m_assembly.appendInstruction(polynomial::Instruction::EQ);
			m_assembly.appendJumpToIf(bodyLabel);
		}
		else
			// default case
			(*this)(c.body);
	}
	m_assembly.setSourceLocation(_switch.location);
	m_assembly.appendJumpTo(end);

	size_t numCases = caseBodies.size();
	for (auto const& c: caseBodies)
	{
		m_assembly.setSourceLocation(c.first->location);
		m_assembly.appendLabel(c.second);
		(*this)(c.first->body);
		// Avoid useless "jump to next" for the last case.
		if (--numCases > 0)
		{
			m_assembly.setSourceLocation(c.first->location);
			m_assembly.appendJumpTo(end);
		}
	}

	m_assembly.setSourceLocation(_switch.location);
	m_assembly.appendLabel(end);
	m_assembly.appendInstruction(polynomial::Instruction::POP);
	checkStackHeight(&_switch);
}

void CodeTransform::operator()(FunctionDefinition const& _function)
{
	polAssert(m_scope, "");
	polAssert(m_scope->identifiers.count(_function.name), "");
	Scope::Function& function = boost::get<Scope::Function>(m_scope->identifiers.at(_function.name));

	int const localStackAdjustment = m_svm15 ? 0 : 1;
	int height = localStackAdjustment;
	polAssert(m_info.scopes.at(&_function.body), "");
	Scope* varScope = m_info.scopes.at(m_info.virtualBlocks.at(&_function).get()).get();
	polAssert(varScope, "");
	for (auto const& v: _function.parameters | boost::adaptors::reversed)
	{
		auto& var = boost::get<Scope::Variable>(varScope->identifiers.at(v.name));
		m_context->variableStackHeights[&var] = height++;
	}

	m_assembly.setSourceLocation(_function.location);
	int stackHeightBefore = m_assembly.stackHeight();
	AbstractAssembly::LabelID afterFunction = m_assembly.newLabelId();

	if (m_svm15)
	{
		m_assembly.appendJumpTo(afterFunction, -stackHeightBefore);
		m_assembly.appendBeginsub(functionEntryID(_function.name, function), _function.parameters.size());
	}
	else
	{
		m_assembly.appendJumpTo(afterFunction, -stackHeightBefore + height);
		m_assembly.appendLabel(functionEntryID(_function.name, function));
	}
	m_stackAdjustment += localStackAdjustment;

	for (auto const& v: _function.returnVariables)
	{
		auto& var = boost::get<Scope::Variable>(varScope->identifiers.at(v.name));
		m_context->variableStackHeights[&var] = height++;
		// Preset stack slots for return variables to zero.
		m_assembly.appendConstant(u256(0));
	}

	CodeTransform(
		m_assembly,
		m_info,
		_function.body,
		m_allowStackOpt,
		m_dialect,
		m_svm15,
		m_identifierAccess,
		m_useNamedLabelsForFunctions,
		localStackAdjustment,
		m_context
	)(_function.body);

	{
		// The stack layout here is:
		// <return label>? <arguments...> <return values...>
		// But we would like it to be:
		// <return values...> <return label>?
		// So we have to append some SWAP and POP instructions.

		// This vector holds the desired target positions of all stack slots and is
		// modified parallel to the actual stack.
		vector<int> stackLayout;
		if (!m_svm15)
			stackLayout.push_back(_function.returnVariables.size()); // Move return label to the top
		stackLayout += vector<int>(_function.parameters.size(), -1); // discard all arguments

		for (size_t i = 0; i < _function.returnVariables.size(); ++i)
			stackLayout.push_back(i); // Move return values down, but keep order.

		polAssert(stackLayout.size() <= 17, "Stack too deep");
		while (!stackLayout.empty() && stackLayout.back() != int(stackLayout.size() - 1))
			if (stackLayout.back() < 0)
			{
				m_assembly.appendInstruction(polynomial::Instruction::POP);
				stackLayout.pop_back();
			}
			else
			{
				m_assembly.appendInstruction(swapInstruction(stackLayout.size() - stackLayout.back() - 1));
				swap(stackLayout[stackLayout.back()], stackLayout.back());
			}
		for (int i = 0; size_t(i) < stackLayout.size(); ++i)
			polAssert(i == stackLayout[i], "Error reshuffling stack.");
	}

	if (m_svm15)
		m_assembly.appendReturnsub(_function.returnVariables.size(), stackHeightBefore);
	else
		m_assembly.appendJump(stackHeightBefore - _function.returnVariables.size());
	m_stackAdjustment -= localStackAdjustment;
	m_assembly.appendLabel(afterFunction);
	checkStackHeight(&_function);
}

void CodeTransform::operator()(ForLoop const& _forLoop)
{
	Scope* originalScope = m_scope;
	// We start with visiting the block, but not finalizing it.
	m_scope = m_info.scopes.at(&_forLoop.pre).get();
	int stackStartHeight = m_assembly.stackHeight();

	visitStatements(_forLoop.pre.statements);

	// TODO: When we implement break and continue, the labels and the stack heights at that point
	// have to be stored in a stack.
	AbstractAssembly::LabelID loopStart = m_assembly.newLabelId();
	AbstractAssembly::LabelID loopEnd = m_assembly.newLabelId();
	AbstractAssembly::LabelID postPart = m_assembly.newLabelId();

	m_assembly.setSourceLocation(_forLoop.location);
	m_assembly.appendLabel(loopStart);

	visitExpression(*_forLoop.condition);
	m_assembly.setSourceLocation(_forLoop.location);
	m_assembly.appendInstruction(polynomial::Instruction::ISZERO);
	m_assembly.appendJumpToIf(loopEnd);

	(*this)(_forLoop.body);

	m_assembly.setSourceLocation(_forLoop.location);
	m_assembly.appendLabel(postPart);

	(*this)(_forLoop.post);

	m_assembly.setSourceLocation(_forLoop.location);
	m_assembly.appendJumpTo(loopStart);
	m_assembly.appendLabel(loopEnd);

	finalizeBlock(_forLoop.pre, stackStartHeight);
	m_scope = originalScope;
}

void CodeTransform::operator()(Block const& _block)
{
	Scope* originalScope = m_scope;
	m_scope = m_info.scopes.at(&_block).get();

	int blockStartStackHeight = m_assembly.stackHeight();
	visitStatements(_block.statements);

	finalizeBlock(_block, blockStartStackHeight);
	m_scope = originalScope;
}

AbstractAssembly::LabelID CodeTransform::labelFromIdentifier(Identifier const& _identifier)
{
	AbstractAssembly::LabelID label = AbstractAssembly::LabelID(-1);
	if (!m_scope->lookup(_identifier.name, Scope::NonconstVisitor(
		[=](Scope::Variable&) { polAssert(false, "Expected label"); },
		[&](Scope::Label& _label)
		{
			label = labelID(_label);
		},
		[=](Scope::Function&) { polAssert(false, "Expected label"); }
	)))
	{
		polAssert(false, "Identifier not found.");
	}
	return label;
}

AbstractAssembly::LabelID CodeTransform::labelID(Scope::Label const& _label)
{
	if (!m_context->labelIDs.count(&_label))
		m_context->labelIDs[&_label] = m_assembly.newLabelId();
	return m_context->labelIDs[&_label];
}

AbstractAssembly::LabelID CodeTransform::functionEntryID(YulString _name, Scope::Function const& _function)
{
	if (!m_context->functionEntryIDs.count(&_function))
	{
		AbstractAssembly::LabelID id =
			m_useNamedLabelsForFunctions ?
			m_assembly.namedLabel(_name.str()) :
			m_assembly.newLabelId();
		m_context->functionEntryIDs[&_function] = id;
	}
	return m_context->functionEntryIDs[&_function];
}

void CodeTransform::visitExpression(Expression const& _expression)
{
	int height = m_assembly.stackHeight();
	boost::apply_visitor(*this, _expression);
	expectDeposit(1, height);
}

void CodeTransform::visitStatements(vector<Statement> const& _statements)
{
	for (auto const& statement: _statements)
	{
		freeUnusedVariables();
		boost::apply_visitor(*this, statement);
	}
	freeUnusedVariables();
}

void CodeTransform::finalizeBlock(Block const& _block, int blockStartStackHeight)
{
	m_assembly.setSourceLocation(_block.location);

	freeUnusedVariables();

	// pop variables
	polAssert(m_info.scopes.at(&_block).get() == m_scope, "");
	for (auto const& id: m_scope->identifiers)
		if (id.second.type() == typeid(Scope::Variable))
		{
			Scope::Variable const& var = boost::get<Scope::Variable>(id.second);
			if (m_allowStackOpt)
			{
				polAssert(!m_context->variableStackHeights.count(&var), "");
				polAssert(!m_context->variableReferences.count(&var), "");
				m_stackAdjustment++;
			}
			else
				m_assembly.appendInstruction(polynomial::Instruction::POP);
		}

	int deposit = m_assembly.stackHeight() - blockStartStackHeight;
	polAssert(deposit == 0, "Invalid stack height at end of block: " + to_string(deposit));
	checkStackHeight(&_block);
}

void CodeTransform::generateMultiAssignment(vector<Identifier> const& _variableNames)
{
	polAssert(m_scope, "");
	for (auto const& variableName: _variableNames | boost::adaptors::reversed)
		generateAssignment(variableName);
}

void CodeTransform::generateAssignment(Identifier const& _variableName)
{
	polAssert(m_scope, "");
	if (auto var = m_scope->lookup(_variableName.name))
	{
		Scope::Variable const& _var = boost::get<Scope::Variable>(*var);
		if (int heightDiff = variableHeightDiff(_var, true))
			m_assembly.appendInstruction(polynomial::swapInstruction(heightDiff - 1));
		m_assembly.appendInstruction(polynomial::Instruction::POP);
		decreaseReference(_variableName.name, _var);
	}
	else
	{
		polAssert(
			m_identifierAccess.generateCode,
			"Identifier not found and no external access available."
		);
		m_identifierAccess.generateCode(_variableName, IdentifierContext::LValue, m_assembly);
	}
}

int CodeTransform::variableHeightDiff(Scope::Variable const& _var, bool _forSwap) const
{
	polAssert(m_context->variableStackHeights.count(&_var), "");
	int heightDiff = m_assembly.stackHeight() - m_context->variableStackHeights[&_var];
	if (heightDiff <= (_forSwap ? 1 : 0) || heightDiff > (_forSwap ? 17 : 16))
	{
		polUnimplemented(
			"Variable inaccessible, too deep inside stack (" + to_string(heightDiff) + ")"
		);
		return 0;
	}
	else
		return heightDiff;
}

void CodeTransform::expectDeposit(int _deposit, int _oldHeight) const
{
	polAssert(m_assembly.stackHeight() == _oldHeight + _deposit, "Invalid stack deposit.");
}

void CodeTransform::checkStackHeight(void const* _astElement) const
{
	polAssert(m_info.stackHeightInfo.count(_astElement), "Stack height for AST element not found.");
	int stackHeightInAnalysis = m_info.stackHeightInfo.at(_astElement);
	int stackHeightInCodegen = m_assembly.stackHeight() - m_stackAdjustment;
	polAssert(
		stackHeightInAnalysis == stackHeightInCodegen,
		"Stack height mismatch between analysis and code generation phase: Analysis: " +
		to_string(stackHeightInAnalysis) +
		" code gen: " +
		to_string(stackHeightInCodegen)
	);
}
