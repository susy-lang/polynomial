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
 * Polynomial AST to SVM bytecode compiler for expressions.
 */

#include <utility>
#include <numeric>
#include <boost/range/adaptor/reversed.hpp>
#include <libdevcore/Common.h>
#include <libdevcore/SHA3.h>
#include <libsofcore/ChainOperationParams.h>
#include <libpolynomial/ast/AST.h>
#include <libpolynomial/codegen/ExpressionCompiler.h>
#include <libpolynomial/codegen/CompilerContext.h>
#include <libpolynomial/codegen/CompilerUtils.h>
#include <libpolynomial/codegen/LValue.h>
using namespace std;

// TODO: FIXME: HOMESTEAD: XXX: @chrissof Params deprecated - use SVMSchedule instead.

namespace dev
{
namespace polynomial
{

void ExpressionCompiler::compile(Expression const& _expression)
{
	_expression.accept(*this);
}

void ExpressionCompiler::appendStateVariableInitialization(VariableDeclaration const& _varDecl)
{
	if (!_varDecl.value())
		return;
	TypePointer type = _varDecl.value()->annotation().type;
	polAssert(!!type, "Type information not available.");
	CompilerContext::LocationSetter locationSetter(m_context, _varDecl);
	_varDecl.value()->accept(*this);

	if (_varDecl.annotation().type->dataStoredIn(DataLocation::Storage))
	{
		// reference type, only convert value to mobile type and do final conversion in storeValue.
		utils().convertType(*type, *type->mobileType());
		type = type->mobileType();
	}
	else
	{
		utils().convertType(*type, *_varDecl.annotation().type);
		type = _varDecl.annotation().type;
	}
	StorageItem(m_context, _varDecl).storeValue(*type, _varDecl.location(), true);
}

void ExpressionCompiler::appendConstStateVariableAccessor(VariableDeclaration const& _varDecl)
{
	polAssert(_varDecl.isConstant(), "");
	_varDecl.value()->accept(*this);
	utils().convertType(*_varDecl.value()->annotation().type, *_varDecl.annotation().type);

	// append return
	m_context << sof::dupInstruction(_varDecl.annotation().type->sizeOnStack() + 1);
	m_context.appendJump(sof::AssemblyItem::JumpType::OutOfFunction);
}

void ExpressionCompiler::appendStateVariableAccessor(VariableDeclaration const& _varDecl)
{
	polAssert(!_varDecl.isConstant(), "");
	CompilerContext::LocationSetter locationSetter(m_context, _varDecl);
	FunctionType accessorType(_varDecl);

	TypePointers paramTypes = accessorType.parameterTypes();

	// retrieve the position of the variable
	auto const& location = m_context.storageLocationOfVariable(_varDecl);
	m_context << location.first << u256(location.second);

	TypePointer returnType = _varDecl.annotation().type;

	for (size_t i = 0; i < paramTypes.size(); ++i)
	{
		if (auto mappingType = dynamic_cast<MappingType const*>(returnType.get()))
		{
			polAssert(CompilerUtils::freeMemoryPointer >= 0x40, "");
			polAssert(
				!paramTypes[i]->isDynamicallySized(),
				"Accessors for mapping with dynamically-sized keys not yet implemented."
			);
			// pop offset
			m_context << sof::Instruction::POP;
			// move storage offset to memory.
			utils().storeInMemory(32);
			// move key to memory.
			utils().copyToStackTop(paramTypes.size() - i, 1);
			utils().storeInMemory(0);
			m_context << u256(64) << u256(0) << sof::Instruction::SHA3;
			// push offset
			m_context << u256(0);
			returnType = mappingType->valueType();
		}
		else if (auto arrayType = dynamic_cast<ArrayType const*>(returnType.get()))
		{
			// pop offset
			m_context << sof::Instruction::POP;
			utils().copyToStackTop(paramTypes.size() - i + 1, 1);
			ArrayUtils(m_context).accessIndex(*arrayType);
			returnType = arrayType->baseType();
		}
		else
			polAssert(false, "Index access is allowed only for \"mapping\" and \"array\" types.");
	}
	// remove index arguments.
	if (paramTypes.size() == 1)
		m_context << sof::Instruction::SWAP2 << sof::Instruction::POP << sof::Instruction::SWAP1;
	else if (paramTypes.size() >= 2)
	{
		m_context << sof::swapInstruction(paramTypes.size());
		m_context << sof::Instruction::POP;
		m_context << sof::swapInstruction(paramTypes.size());
		utils().popStackSlots(paramTypes.size() - 1);
	}
	unsigned retSizeOnStack = 0;
	polAssert(accessorType.returnParameterTypes().size() >= 1, "");
	auto const& returnTypes = accessorType.returnParameterTypes();
	if (StructType const* structType = dynamic_cast<StructType const*>(returnType.get()))
	{
		// remove offset
		m_context << sof::Instruction::POP;
		auto const& names = accessorType.returnParameterNames();
		// struct
		for (size_t i = 0; i < names.size(); ++i)
		{
			if (returnTypes[i]->category() == Type::Category::Mapping)
				continue;
			if (auto arrayType = dynamic_cast<ArrayType const*>(returnTypes[i].get()))
				if (!arrayType->isByteArray())
					continue;
			pair<u256, unsigned> const& offsets = structType->storageOffsetsOfMember(names[i]);
			m_context << sof::Instruction::DUP1 << u256(offsets.first) << sof::Instruction::ADD << u256(offsets.second);
			TypePointer memberType = structType->memberType(names[i]);
			StorageItem(m_context, *memberType).retrieveValue(SourceLocation(), true);
			utils().convertType(*memberType, *returnTypes[i]);
			utils().moveToStackTop(returnTypes[i]->sizeOnStack());
			retSizeOnStack += returnTypes[i]->sizeOnStack();
		}
		// remove slot
		m_context << sof::Instruction::POP;
	}
	else
	{
		// simple value or array
		polAssert(returnTypes.size() == 1, "");
		StorageItem(m_context, *returnType).retrieveValue(SourceLocation(), true);
		utils().convertType(*returnType, *returnTypes.front());
		retSizeOnStack = returnTypes.front()->sizeOnStack();
	}
	polAssert(retSizeOnStack == utils().sizeOnStack(returnTypes), "");
	polAssert(retSizeOnStack <= 15, "Stack is too deep.");
	m_context << sof::dupInstruction(retSizeOnStack + 1);
	m_context.appendJump(sof::AssemblyItem::JumpType::OutOfFunction);
}

bool ExpressionCompiler::visit(Conditional const& _condition)
{
	CompilerContext::LocationSetter locationSetter(m_context, _condition);
	_condition.condition().accept(*this);
	sof::AssemblyItem trueTag = m_context.appendConditionalJump();
	_condition.falseExpression().accept(*this);
	utils().convertType(*_condition.falseExpression().annotation().type, *_condition.annotation().type);
	sof::AssemblyItem endTag = m_context.appendJumpToNew();
	m_context << trueTag;
	int offset = _condition.annotation().type->sizeOnStack();
	m_context.adjustStackOffset(-offset);
	_condition.trueExpression().accept(*this);
	utils().convertType(*_condition.trueExpression().annotation().type, *_condition.annotation().type);
	m_context << endTag;
	return false;
}

bool ExpressionCompiler::visit(Assignment const& _assignment)
{
	CompilerContext::LocationSetter locationSetter(m_context, _assignment);
	_assignment.rightHandSide().accept(*this);
	// Perform some conversion already. This will convert storage types to memory and literals
	// to their actual type, but will not convert e.g. memory to storage.
	TypePointer type = _assignment.rightHandSide().annotation().type->closestTemporaryType(
		_assignment.leftHandSide().annotation().type
	);
	utils().convertType(*_assignment.rightHandSide().annotation().type, *type);

	_assignment.leftHandSide().accept(*this);
	polAssert(!!m_currentLValue, "LValue not retrieved.");

	Token::Value op = _assignment.assignmentOperator();
	if (op != Token::Assign) // compound assignment
	{
		polAssert(_assignment.annotation().type->isValueType(), "Compound operators not implemented for non-value types.");
		unsigned lvalueSize = m_currentLValue->sizeOnStack();
		unsigned itemSize = _assignment.annotation().type->sizeOnStack();
		if (lvalueSize > 0)
		{
			utils().copyToStackTop(lvalueSize + itemSize, itemSize);
			utils().copyToStackTop(itemSize + lvalueSize, lvalueSize);
			// value lvalue_ref value lvalue_ref
		}
		m_currentLValue->retrieveValue(_assignment.location(), true);
		appendOrdinaryBinaryOperatorCode(Token::AssignmentToBinaryOp(op), *_assignment.annotation().type);
		if (lvalueSize > 0)
		{
			polAssert(itemSize + lvalueSize <= 16, "Stack too deep, try removing local variables.");
			// value [lvalue_ref] updated_value
			for (unsigned i = 0; i < itemSize; ++i)
				m_context << sof::swapInstruction(itemSize + lvalueSize) << sof::Instruction::POP;
		}
	}
	m_currentLValue->storeValue(*type, _assignment.location());
	m_currentLValue.reset();
	return false;
}

bool ExpressionCompiler::visit(TupleExpression const& _tuple)
{
	if (_tuple.isInlineArray())
	{
		ArrayType const& arrayType = dynamic_cast<ArrayType const&>(*_tuple.annotation().type);
		
		polAssert(!arrayType.isDynamicallySized(), "Cannot create dynamically sized inline array.");
		m_context << max(u256(32u), arrayType.memorySize());
		utils().allocateMemory();
		m_context << sof::Instruction::DUP1;
	
		for (auto const& component: _tuple.components())
		{
			component->accept(*this);
			utils().convertType(*component->annotation().type, *arrayType.baseType(), true);
			utils().storeInMemoryDynamic(*arrayType.baseType(), true);				
		}
		
		m_context << sof::Instruction::POP;
	}
	else
	{
		vector<unique_ptr<LValue>> lvalues;
		for (auto const& component: _tuple.components())
			if (component)
			{
				component->accept(*this);
				if (_tuple.annotation().lValueRequested)
				{
					polAssert(!!m_currentLValue, "");
					lvalues.push_back(move(m_currentLValue));
				}
			}
			else if (_tuple.annotation().lValueRequested)
				lvalues.push_back(unique_ptr<LValue>());
		if (_tuple.annotation().lValueRequested)
		{
			if (_tuple.components().size() == 1)
				m_currentLValue = move(lvalues[0]);
			else
				m_currentLValue.reset(new TupleObject(m_context, move(lvalues)));
		}
	}
	return false;
}

bool ExpressionCompiler::visit(UnaryOperation const& _unaryOperation)
{
	CompilerContext::LocationSetter locationSetter(m_context, _unaryOperation);
	//@todo type checking and creating code for an operator should be in the same place:
	// the operator should know how to convert itself and to which types it applies, so
	// put this code together with "Type::acceptsBinary/UnaryOperator" into a class that
	// represents the operator
	if (_unaryOperation.annotation().type->category() == Type::Category::IntegerConstant)
	{
		m_context << _unaryOperation.annotation().type->literalValue(nullptr);
		return false;
	}

	_unaryOperation.subExpression().accept(*this);

	switch (_unaryOperation.getOperator())
	{
	case Token::Not: // !
		m_context << sof::Instruction::ISZERO;
		break;
	case Token::BitNot: // ~
		m_context << sof::Instruction::NOT;
		break;
	case Token::After: // after
		m_context << sof::Instruction::TIMESTAMP << sof::Instruction::ADD;
		break;
	case Token::Delete: // delete
		polAssert(!!m_currentLValue, "LValue not retrieved.");
		m_currentLValue->setToZero(_unaryOperation.location());
		m_currentLValue.reset();
		break;
	case Token::Inc: // ++ (pre- or postfix)
	case Token::Dec: // -- (pre- or postfix)
		polAssert(!!m_currentLValue, "LValue not retrieved.");
		m_currentLValue->retrieveValue(_unaryOperation.location());
		if (!_unaryOperation.isPrefixOperation())
		{
			// store value for later
			polAssert(_unaryOperation.annotation().type->sizeOnStack() == 1, "Stack size != 1 not implemented.");
			m_context << sof::Instruction::DUP1;
			if (m_currentLValue->sizeOnStack() > 0)
				for (unsigned i = 1 + m_currentLValue->sizeOnStack(); i > 0; --i)
					m_context << sof::swapInstruction(i);
		}
		m_context << u256(1);
		if (_unaryOperation.getOperator() == Token::Inc)
			m_context << sof::Instruction::ADD;
		else
			m_context << sof::Instruction::SWAP1 << sof::Instruction::SUB;
		// Stack for prefix: [ref...] (*ref)+-1
		// Stack for postfix: *ref [ref...] (*ref)+-1
		for (unsigned i = m_currentLValue->sizeOnStack(); i > 0; --i)
			m_context << sof::swapInstruction(i);
		m_currentLValue->storeValue(
			*_unaryOperation.annotation().type, _unaryOperation.location(),
			!_unaryOperation.isPrefixOperation());
		m_currentLValue.reset();
		break;
	case Token::Add: // +
		// unary add, so basically no-op
		break;
	case Token::Sub: // -
		m_context << u256(0) << sof::Instruction::SUB;
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Invalid unary operator: " +
																		 string(Token::toString(_unaryOperation.getOperator()))));
	}
	return false;
}

bool ExpressionCompiler::visit(BinaryOperation const& _binaryOperation)
{
	CompilerContext::LocationSetter locationSetter(m_context, _binaryOperation);
	Expression const& leftExpression = _binaryOperation.leftExpression();
	Expression const& rightExpression = _binaryOperation.rightExpression();
	polAssert(!!_binaryOperation.annotation().commonType, "");
	Type const& commonType = *_binaryOperation.annotation().commonType;
	Token::Value const c_op = _binaryOperation.getOperator();

	if (c_op == Token::And || c_op == Token::Or) // special case: short-circuiting
		appendAndOrOperatorCode(_binaryOperation);
	else if (commonType.category() == Type::Category::IntegerConstant)
		m_context << commonType.literalValue(nullptr);
	else
	{
		bool cleanupNeeded = commonType.category() == Type::Category::Integer &&
			(Token::isCompareOp(c_op) || c_op == Token::Div || c_op == Token::Mod);

		// for commutative operators, push the literal as late as possible to allow improved optimization
		auto isLiteral = [](Expression const& _e)
		{
			return dynamic_cast<Literal const*>(&_e) || _e.annotation().type->category() == Type::Category::IntegerConstant;
		};
		bool swap = m_optimize && Token::isCommutativeOp(c_op) && isLiteral(rightExpression) && !isLiteral(leftExpression);
		if (swap)
		{
			leftExpression.accept(*this);
			utils().convertType(*leftExpression.annotation().type, commonType, cleanupNeeded);
			rightExpression.accept(*this);
			utils().convertType(*rightExpression.annotation().type, commonType, cleanupNeeded);
		}
		else
		{
			rightExpression.accept(*this);
			utils().convertType(*rightExpression.annotation().type, commonType, cleanupNeeded);
			leftExpression.accept(*this);
			utils().convertType(*leftExpression.annotation().type, commonType, cleanupNeeded);
		}
		if (Token::isCompareOp(c_op))
			appendCompareOperatorCode(c_op, commonType);
		else
			appendOrdinaryBinaryOperatorCode(c_op, commonType);
	}

	// do not visit the child nodes, we already did that explicitly
	return false;
}

bool ExpressionCompiler::visit(FunctionCall const& _functionCall)
{
	CompilerContext::LocationSetter locationSetter(m_context, _functionCall);
	using Location = FunctionType::Location;
	if (_functionCall.annotation().isTypeConversion)
	{
		polAssert(_functionCall.arguments().size() == 1, "");
		polAssert(_functionCall.names().empty(), "");
		Expression const& firstArgument = *_functionCall.arguments().front();
		firstArgument.accept(*this);
		utils().convertType(*firstArgument.annotation().type, *_functionCall.annotation().type);
		return false;
	}

	FunctionTypePointer functionType;
	if (_functionCall.annotation().isStructConstructorCall)
	{
		auto const& type = dynamic_cast<TypeType const&>(*_functionCall.expression().annotation().type);
		auto const& structType = dynamic_cast<StructType const&>(*type.actualType());
		functionType = structType.constructorType();
	}
	else
		functionType = dynamic_pointer_cast<FunctionType const>(_functionCall.expression().annotation().type);

	TypePointers parameterTypes = functionType->parameterTypes();
	vector<ASTPointer<Expression const>> const& callArguments = _functionCall.arguments();
	vector<ASTPointer<ASTString>> const& callArgumentNames = _functionCall.names();
	if (!functionType->takesArbitraryParameters())
		polAssert(callArguments.size() == parameterTypes.size(), "");

	vector<ASTPointer<Expression const>> arguments;
	if (callArgumentNames.empty())
		// normal arguments
		arguments = callArguments;
	else
		// named arguments
		for (auto const& parameterName: functionType->parameterNames())
		{
			bool found = false;
			for (size_t j = 0; j < callArgumentNames.size() && !found; j++)
				if ((found = (parameterName == *callArgumentNames[j])))
					// we found the actual parameter position
					arguments.push_back(callArguments[j]);
			polAssert(found, "");
		}

	if (_functionCall.annotation().isStructConstructorCall)
	{
		TypeType const& type = dynamic_cast<TypeType const&>(*_functionCall.expression().annotation().type);
		auto const& structType = dynamic_cast<StructType const&>(*type.actualType());

		m_context << max(u256(32u), structType.memorySize());
		utils().allocateMemory();
		m_context << sof::Instruction::DUP1;

		for (unsigned i = 0; i < arguments.size(); ++i)
		{
			arguments[i]->accept(*this);
			utils().convertType(*arguments[i]->annotation().type, *functionType->parameterTypes()[i]);
			utils().storeInMemoryDynamic(*functionType->parameterTypes()[i]);
		}
		m_context << sof::Instruction::POP;
	}
	else
	{
		FunctionType const& function = *functionType;
		if (function.bound())
			// Only delegatecall functions can be bound, this might be lifted later.
			polAssert(function.location() == Location::DelegateCall, "");
		switch (function.location())
		{
		case Location::Internal:
		{
			// Calling convention: Caller pushes return address and arguments
			// Callee removes them and pushes return values

			sof::AssemblyItem returnLabel = m_context.pushNewTag();
			for (unsigned i = 0; i < arguments.size(); ++i)
			{
				arguments[i]->accept(*this);
				utils().convertType(*arguments[i]->annotation().type, *function.parameterTypes()[i]);
			}
			_functionCall.expression().accept(*this);

			m_context.appendJump(sof::AssemblyItem::JumpType::IntoFunction);
			m_context << returnLabel;

			unsigned returnParametersSize = CompilerUtils::sizeOnStack(function.returnParameterTypes());
			// callee adds return parameters, but removes arguments and return label
			m_context.adjustStackOffset(returnParametersSize - CompilerUtils::sizeOnStack(function.parameterTypes()) - 1);
			break;
		}
		case Location::External:
		case Location::CallCode:
		case Location::DelegateCall:
		case Location::Bare:
		case Location::BareCallCode:
		case Location::BareDelegateCall:
			_functionCall.expression().accept(*this);
			appendExternalFunctionCall(function, arguments);
			break;
		case Location::Creation:
		{
			_functionCall.expression().accept(*this);
			polAssert(!function.gasSet(), "Gas limit set for contract creation.");
			polAssert(function.returnParameterTypes().size() == 1, "");
			TypePointers argumentTypes;
			for (auto const& arg: arguments)
			{
				arg->accept(*this);
				argumentTypes.push_back(arg->annotation().type);
			}
			ContractDefinition const& contract =
				dynamic_cast<ContractType const&>(*function.returnParameterTypes().front()).contractDefinition();
			// copy the contract's code into memory
			sof::Assembly const& assembly = m_context.compiledContract(contract);
			utils().fetchFreeMemoryPointer();
			// pushes size
			sof::AssemblyItem subroutine = m_context.addSubroutine(assembly);
			m_context << sof::Instruction::DUP1 << subroutine;
			m_context << sof::Instruction::DUP4 << sof::Instruction::CODECOPY;

			m_context << sof::Instruction::ADD;
			utils().encodeToMemory(argumentTypes, function.parameterTypes());
			// now on stack: memory_end_ptr
			// need: size, offset, endowment
			utils().toSizeAfterFreeMemoryPointer();
			if (function.valueSet())
				m_context << sof::dupInstruction(3);
			else
				m_context << u256(0);
			m_context << sof::Instruction::CREATE;
			if (function.valueSet())
				m_context << sof::swapInstruction(1) << sof::Instruction::POP;
			break;
		}
		case Location::SetGas:
		{
			// stack layout: contract_address function_id [gas] [value]
			_functionCall.expression().accept(*this);

			arguments.front()->accept(*this);
			utils().convertType(*arguments.front()->annotation().type, IntegerType(256), true);
			// Note that function is not the original function, but the ".gas" function.
			// Its values of gasSet and valueSet is equal to the original function's though.
			unsigned stackDepth = (function.gasSet() ? 1 : 0) + (function.valueSet() ? 1 : 0);
			if (stackDepth > 0)
				m_context << sof::swapInstruction(stackDepth);
			if (function.gasSet())
				m_context << sof::Instruction::POP;
			break;
		}
		case Location::SetValue:
			// stack layout: contract_address function_id [gas] [value]
			_functionCall.expression().accept(*this);
			// Note that function is not the original function, but the ".value" function.
			// Its values of gasSet and valueSet is equal to the original function's though.
			if (function.valueSet())
				m_context << sof::Instruction::POP;
			arguments.front()->accept(*this);
			break;
		case Location::Send:
			_functionCall.expression().accept(*this);
			m_context << u256(0); // do not send gas (there still is the stipend)
			arguments.front()->accept(*this);
			utils().convertType(
				*arguments.front()->annotation().type,
				*function.parameterTypes().front(), true
			);
			appendExternalFunctionCall(
				FunctionType(
					TypePointers{},
					TypePointers{},
					strings(),
					strings(),
					Location::Bare,
					false,
					nullptr,
					true,
					true
				),
				{}
			);
			break;
		case Location::Selfdestruct:
			arguments.front()->accept(*this);
			utils().convertType(*arguments.front()->annotation().type, *function.parameterTypes().front(), true);
			m_context << sof::Instruction::SUICIDE;
			break;
		case Location::SHA3:
		{
			TypePointers argumentTypes;
			for (auto const& arg: arguments)
			{
				arg->accept(*this);
				argumentTypes.push_back(arg->annotation().type);
			}
			utils().fetchFreeMemoryPointer();
			utils().encodeToMemory(argumentTypes, TypePointers(), function.padArguments(), true);
			utils().toSizeAfterFreeMemoryPointer();
			m_context << sof::Instruction::SHA3;
			break;
		}
		case Location::Log0:
		case Location::Log1:
		case Location::Log2:
		case Location::Log3:
		case Location::Log4:
		{
			unsigned logNumber = int(function.location()) - int(Location::Log0);
			for (unsigned arg = logNumber; arg > 0; --arg)
			{
				arguments[arg]->accept(*this);
				utils().convertType(*arguments[arg]->annotation().type, *function.parameterTypes()[arg], true);
			}
			arguments.front()->accept(*this);
			utils().fetchFreeMemoryPointer();
			utils().encodeToMemory(
				{arguments.front()->annotation().type},
				{function.parameterTypes().front()},
				false,
				true);
			utils().toSizeAfterFreeMemoryPointer();
			m_context << sof::logInstruction(logNumber);
			break;
		}
		case Location::Event:
		{
			_functionCall.expression().accept(*this);
			auto const& event = dynamic_cast<EventDefinition const&>(function.declaration());
			unsigned numIndexed = 0;
			// All indexed arguments go to the stack
			for (unsigned arg = arguments.size(); arg > 0; --arg)
				if (event.parameters()[arg - 1]->isIndexed())
				{
					++numIndexed;
					arguments[arg - 1]->accept(*this);
					if (auto const& arrayType = dynamic_pointer_cast<ArrayType const>(function.parameterTypes()[arg - 1]))
					{
						utils().fetchFreeMemoryPointer();
						utils().encodeToMemory(
							{arguments[arg - 1]->annotation().type},
							{arrayType},
							false,
							true
						);
						utils().toSizeAfterFreeMemoryPointer();
						m_context << sof::Instruction::SHA3;
					}
					else
						utils().convertType(
							*arguments[arg - 1]->annotation().type,
							*function.parameterTypes()[arg - 1],
							true
						);
				}
			if (!event.isAnonymous())
			{
				m_context << u256(h256::Arith(dev::sha3(function.externalSignature())));
				++numIndexed;
			}
			polAssert(numIndexed <= 4, "Too many indexed arguments.");
			// Copy all non-indexed arguments to memory (data)
			// Memory position is only a hack and should be removed once we have free memory pointer.
			TypePointers nonIndexedArgTypes;
			TypePointers nonIndexedParamTypes;
			for (unsigned arg = 0; arg < arguments.size(); ++arg)
				if (!event.parameters()[arg]->isIndexed())
				{
					arguments[arg]->accept(*this);
					nonIndexedArgTypes.push_back(arguments[arg]->annotation().type);
					nonIndexedParamTypes.push_back(function.parameterTypes()[arg]);
				}
			utils().fetchFreeMemoryPointer();
			utils().encodeToMemory(nonIndexedArgTypes, nonIndexedParamTypes);
			// need: topic1 ... topicn memsize memstart
			utils().toSizeAfterFreeMemoryPointer();
			m_context << sof::logInstruction(numIndexed);
			break;
		}
		case Location::BlockHash:
		{
			arguments[0]->accept(*this);
			utils().convertType(*arguments[0]->annotation().type, *function.parameterTypes()[0], true);
			m_context << sof::Instruction::BLOCKHASH;
			break;
		}
		case Location::AddMod:
		case Location::MulMod:
		{
			for (unsigned i = 0; i < 3; i ++)
			{
				arguments[2 - i]->accept(*this);
				utils().convertType(*arguments[2 - i]->annotation().type, IntegerType(256));
			}
			if (function.location() == Location::AddMod)
				m_context << sof::Instruction::ADDMOD;
			else
				m_context << sof::Instruction::MULMOD;
			break;
		}
		case Location::ECRecover:
		case Location::SHA256:
		case Location::RIPEMD160:
		{
			_functionCall.expression().accept(*this);
			static const map<Location, u256> contractAddresses{{Location::ECRecover, 1},
															   {Location::SHA256, 2},
															   {Location::RIPEMD160, 3}};
			m_context << contractAddresses.find(function.location())->second;
			for (unsigned i = function.sizeOnStack(); i > 0; --i)
				m_context << sof::swapInstruction(i);
			appendExternalFunctionCall(function, arguments);
			break;
		}
		case Location::ByteArrayPush:
		case Location::ArrayPush:
		{
			_functionCall.expression().accept(*this);
			polAssert(function.parameterTypes().size() == 1, "");
			polAssert(!!function.parameterTypes()[0], "");
			TypePointer paramType = function.parameterTypes()[0];
			shared_ptr<ArrayType> arrayType =
				function.location() == Location::ArrayPush ?
				make_shared<ArrayType>(DataLocation::Storage, paramType) :
				make_shared<ArrayType>(DataLocation::Storage);
			// get the current length
			ArrayUtils(m_context).retrieveLength(*arrayType);
			m_context << sof::Instruction::DUP1;
			// stack: ArrayReference currentLength currentLength
			m_context << u256(1) << sof::Instruction::ADD;
			// stack: ArrayReference currentLength newLength
			m_context << sof::Instruction::DUP3 << sof::Instruction::DUP2;
			ArrayUtils(m_context).resizeDynamicArray(*arrayType);
			m_context << sof::Instruction::SWAP2 << sof::Instruction::SWAP1;
			// stack: newLength ArrayReference oldLength
			ArrayUtils(m_context).accessIndex(*arrayType, false);

			// stack: newLength storageSlot slotOffset
			arguments[0]->accept(*this);
			// stack: newLength storageSlot slotOffset argValue
			TypePointer type = arguments[0]->annotation().type->closestTemporaryType(arrayType->baseType());
			utils().convertType(*arguments[0]->annotation().type, *type);
			utils().moveToStackTop(1 + type->sizeOnStack());
			utils().moveToStackTop(1 + type->sizeOnStack());
			// stack: newLength argValue storageSlot slotOffset
			if (function.location() == Location::ArrayPush)
				StorageItem(m_context, *paramType).storeValue(*type, _functionCall.location(), true);
			else
				StorageByteArrayElement(m_context).storeValue(*type, _functionCall.location(), true);
			break;
		}
		case Location::ObjectCreation:
		{
			// Will allocate at the end of memory (MSIZE) and not write at all unless the base
			// type is dynamically sized.
			ArrayType const& arrayType = dynamic_cast<ArrayType const&>(*_functionCall.annotation().type);
			_functionCall.expression().accept(*this);
			polAssert(arguments.size() == 1, "");

			// Fetch requested length.
			arguments[0]->accept(*this);
			utils().convertType(*arguments[0]->annotation().type, IntegerType(256));

			// Stack: requested_length
			// Allocate at max(MSIZE, freeMemoryPointer)
			utils().fetchFreeMemoryPointer();
			m_context << sof::Instruction::DUP1 << sof::Instruction::MSIZE;
			m_context << sof::Instruction::LT;
			auto initialise = m_context.appendConditionalJump();
			// Free memory pointer does not point to empty memory, use MSIZE.
			m_context << sof::Instruction::POP;
			m_context << sof::Instruction::MSIZE;
			m_context << initialise;

			// Stack: requested_length memptr
			m_context << sof::Instruction::SWAP1;
			// Stack: memptr requested_length
			// store length
			m_context << sof::Instruction::DUP1 << sof::Instruction::DUP3 << sof::Instruction::MSTORE;
			// Stack: memptr requested_length
			// update free memory pointer
			m_context << sof::Instruction::DUP1 << arrayType.baseType()->memoryHeadSize();
			m_context << sof::Instruction::MUL << u256(32) << sof::Instruction::ADD;
			m_context << sof::Instruction::DUP3 << sof::Instruction::ADD;
			utils().storeFreeMemoryPointer();
			// Stack: memptr requested_length

			// We only have to initialise if the base type is a not a value type.
			if (dynamic_cast<ReferenceType const*>(arrayType.baseType().get()))
			{
				m_context << sof::Instruction::DUP2 << u256(32) << sof::Instruction::ADD;
				utils().zeroInitialiseMemoryArray(arrayType);
				m_context << sof::Instruction::POP;
			}
			else
				m_context << sof::Instruction::POP;
			break;
		}
		default:
			BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Invalid function type."));
		}
	}
	return false;
}

bool ExpressionCompiler::visit(NewExpression const&)
{
	// code is created for the function call (CREATION) only
	return false;
}

void ExpressionCompiler::endVisit(MemberAccess const& _memberAccess)
{
	CompilerContext::LocationSetter locationSetter(m_context, _memberAccess);
	// Check whether the member is a bound function.
	ASTString const& member = _memberAccess.memberName();
	if (auto funType = dynamic_cast<FunctionType const*>(_memberAccess.annotation().type.get()))
		if (funType->bound())
		{
			utils().convertType(
				*_memberAccess.expression().annotation().type,
				*funType->selfType(),
				true
			);
			auto contract = dynamic_cast<ContractDefinition const*>(funType->declaration().scope());
			polAssert(contract && contract->isLibrary(), "");
			m_context.appendLibraryAddress(contract->name());
			m_context << funType->externalIdentifier();
			utils().moveIntoStack(funType->selfType()->sizeOnStack(), 2);
			return;
		}

	switch (_memberAccess.expression().annotation().type->category())
	{
	case Type::Category::Contract:
	{
		bool alsoSearchInteger = false;
		ContractType const& type = dynamic_cast<ContractType const&>(*_memberAccess.expression().annotation().type);
		if (type.isSuper())
		{
			polAssert(!!_memberAccess.annotation().referencedDeclaration, "Referenced declaration not resolved.");
			m_context << m_context.superFunctionEntryLabel(
				dynamic_cast<FunctionDefinition const&>(*_memberAccess.annotation().referencedDeclaration),
				type.contractDefinition()
			).pushTag();
		}
		else
		{
			// ordinary contract type
			if (Declaration const* declaration = _memberAccess.annotation().referencedDeclaration)
			{
				u256 identifier;
				if (auto const* variable = dynamic_cast<VariableDeclaration const*>(declaration))
					identifier = FunctionType(*variable).externalIdentifier();
				else if (auto const* function = dynamic_cast<FunctionDefinition const*>(declaration))
					identifier = FunctionType(*function).externalIdentifier();
				else
					polAssert(false, "Contract member is neither variable nor function.");
				utils().convertType(type, IntegerType(0, IntegerType::Modifier::Address), true);
				m_context << identifier;
			}
			else
				// not found in contract, search in members inherited from address
				alsoSearchInteger = true;
		}
		if (!alsoSearchInteger)
			break;
	}
	case Type::Category::Integer:
		if (member == "balance")
		{
			utils().convertType(
				*_memberAccess.expression().annotation().type,
				IntegerType(0, IntegerType::Modifier::Address),
				true
			);
			m_context << sof::Instruction::BALANCE;
		}
		else if ((set<string>{"send", "call", "callcode", "delegatecall"}).count(member))
			utils().convertType(
				*_memberAccess.expression().annotation().type,
				IntegerType(0, IntegerType::Modifier::Address),
				true
			);
		else
			BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Invalid member access to integer."));
		break;
	case Type::Category::Function:
		polAssert(!!_memberAccess.expression().annotation().type->memberType(member),
				 "Invalid member access to function.");
		break;
	case Type::Category::Magic:
		// we can ignore the kind of magic and only look at the name of the member
		if (member == "coinbase")
			m_context << sof::Instruction::COINBASE;
		else if (member == "timestamp")
			m_context << sof::Instruction::TIMESTAMP;
		else if (member == "difficulty")
			m_context << sof::Instruction::DIFFICULTY;
		else if (member == "number")
			m_context << sof::Instruction::NUMBER;
		else if (member == "gaslimit")
			m_context << sof::Instruction::GASLIMIT;
		else if (member == "sender")
			m_context << sof::Instruction::CALLER;
		else if (member == "value")
			m_context << sof::Instruction::CALLVALUE;
		else if (member == "origin")
			m_context << sof::Instruction::ORIGIN;
		else if (member == "gas")
			m_context << sof::Instruction::GAS;
		else if (member == "gasprice")
			m_context << sof::Instruction::GASPRICE;
		else if (member == "data")
			m_context << u256(0) << sof::Instruction::CALLDATASIZE;
		else if (member == "sig")
			m_context << u256(0) << sof::Instruction::CALLDATALOAD
				<< (u256(0xffffffff) << (256 - 32)) << sof::Instruction::AND;
		else
			BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown magic member."));
		break;
	case Type::Category::Struct:
	{
		StructType const& type = dynamic_cast<StructType const&>(*_memberAccess.expression().annotation().type);
		switch (type.location())
		{
		case DataLocation::Storage:
		{
			pair<u256, unsigned> const& offsets = type.storageOffsetsOfMember(member);
			m_context << offsets.first << sof::Instruction::ADD << u256(offsets.second);
			setLValueToStorageItem(_memberAccess);
			break;
		}
		case DataLocation::Memory:
		{
			m_context << type.memoryOffsetOfMember(member) << sof::Instruction::ADD;
			setLValue<MemoryItem>(_memberAccess, *_memberAccess.annotation().type);
			break;
		}
		default:
			polAssert(false, "Illegal data location for struct.");
		}
		break;
	}
	case Type::Category::Enum:
	{
		EnumType const& type = dynamic_cast<EnumType const&>(*_memberAccess.expression().annotation().type);
		m_context << type.memberValue(_memberAccess.memberName());
		break;
	}
	case Type::Category::TypeType:
	{
		TypeType const& type = dynamic_cast<TypeType const&>(*_memberAccess.expression().annotation().type);

		if (dynamic_cast<ContractType const*>(type.actualType().get()))
		{
			if (auto funType = dynamic_cast<FunctionType const*>(_memberAccess.annotation().type.get()))
			{
				if (funType->location() != FunctionType::Location::Internal)
					m_context << funType->externalIdentifier();
				else
				{
					auto const* function = dynamic_cast<FunctionDefinition const*>(_memberAccess.annotation().referencedDeclaration);
					polAssert(!!function, "Function not found in member access");
					m_context << m_context.functionEntryLabel(*function).pushTag();
				}
			}
		}
		else if (auto enumType = dynamic_cast<EnumType const*>(type.actualType().get()))
			m_context << enumType->memberValue(_memberAccess.memberName());
		break;
	}
	case Type::Category::Array:
	{
		auto const& type = dynamic_cast<ArrayType const&>(*_memberAccess.expression().annotation().type);
		if (member == "length")
		{
			if (!type.isDynamicallySized())
			{
				utils().popStackElement(type);
				m_context << type.length();
			}
			else
				switch (type.location())
				{
				case DataLocation::CallData:
					m_context << sof::Instruction::SWAP1 << sof::Instruction::POP;
					break;
				case DataLocation::Storage:
					setLValue<StorageArrayLength>(_memberAccess, type);
					break;
				case DataLocation::Memory:
					m_context << sof::Instruction::MLOAD;
					break;
				}
		}
		else if (member == "push")
		{
			polAssert(
				type.isDynamicallySized() && type.location() == DataLocation::Storage,
				"Tried to use .push() on a non-dynamically sized array"
			);
		}
		else
			polAssert(false, "Illegal array member.");
		break;
	}
	case Type::Category::FixedBytes:
	{
		auto const& type = dynamic_cast<FixedBytesType const&>(*_memberAccess.expression().annotation().type);
		utils().popStackElement(type);
		if (member == "length")
			m_context << u256(type.numBytes());
		else
			polAssert(false, "Illegal fixed bytes member.");
		break;
	}
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Member access to unknown type."));
	}
}

bool ExpressionCompiler::visit(IndexAccess const& _indexAccess)
{
	CompilerContext::LocationSetter locationSetter(m_context, _indexAccess);
	_indexAccess.baseExpression().accept(*this);

	Type const& baseType = *_indexAccess.baseExpression().annotation().type;

	if (baseType.category() == Type::Category::Mapping)
	{
		// stack: storage_base_ref
		TypePointer keyType = dynamic_cast<MappingType const&>(baseType).keyType();
		polAssert(_indexAccess.indexExpression(), "Index expression expected.");
		if (keyType->isDynamicallySized())
		{
			_indexAccess.indexExpression()->accept(*this);
			utils().fetchFreeMemoryPointer();
			// stack: base index mem
			// note: the following operations must not allocate memory!
			utils().encodeToMemory(
				TypePointers{_indexAccess.indexExpression()->annotation().type},
				TypePointers{keyType},
				false,
				true
			);
			m_context << sof::Instruction::SWAP1;
			utils().storeInMemoryDynamic(IntegerType(256));
			utils().toSizeAfterFreeMemoryPointer();
		}
		else
		{
			m_context << u256(0); // memory position
			appendExpressionCopyToMemory(*keyType, *_indexAccess.indexExpression());
			m_context << sof::Instruction::SWAP1;
			polAssert(CompilerUtils::freeMemoryPointer >= 0x40, "");
			utils().storeInMemoryDynamic(IntegerType(256));
			m_context << u256(0);
		}
		m_context << sof::Instruction::SHA3;
		m_context << u256(0);
		setLValueToStorageItem(_indexAccess);
	}
	else if (baseType.category() == Type::Category::Array)
	{
		ArrayType const& arrayType = dynamic_cast<ArrayType const&>(baseType);
		polAssert(_indexAccess.indexExpression(), "Index expression expected.");

		_indexAccess.indexExpression()->accept(*this);
		utils().convertType(*_indexAccess.indexExpression()->annotation().type, IntegerType(256), true);
		// stack layout: <base_ref> [<length>] <index>
		ArrayUtils(m_context).accessIndex(arrayType);
		switch (arrayType.location())
		{
		case DataLocation::Storage:
			if (arrayType.isByteArray())
			{
				polAssert(!arrayType.isString(), "Index access to string is not allowed.");
				setLValue<StorageByteArrayElement>(_indexAccess);
			}
			else
				setLValueToStorageItem(_indexAccess);
			break;
		case DataLocation::Memory:
			setLValue<MemoryItem>(_indexAccess, *_indexAccess.annotation().type, !arrayType.isByteArray());
			break;
		case DataLocation::CallData:
			//@todo if we implement this, the value in calldata has to be added to the base offset
			polAssert(!arrayType.baseType()->isDynamicallySized(), "Nested arrays not yet implemented.");
			if (arrayType.baseType()->isValueType())
				CompilerUtils(m_context).loadFromMemoryDynamic(
					*arrayType.baseType(),
					true,
					!arrayType.isByteArray(),
					false
				);
			break;
		}
	}
	else if (baseType.category() == Type::Category::FixedBytes)
	{
		FixedBytesType const& fixedBytesType = dynamic_cast<FixedBytesType const&>(baseType);
		polAssert(_indexAccess.indexExpression(), "Index expression expected.");

		_indexAccess.indexExpression()->accept(*this);
		utils().convertType(*_indexAccess.indexExpression()->annotation().type, IntegerType(256), true);
		// stack layout: <value> <index>
		// check out-of-bounds access
		m_context << u256(fixedBytesType.numBytes());
		m_context << sof::Instruction::DUP2 << sof::Instruction::LT << sof::Instruction::ISZERO;
		// out-of-bounds access throws exception
		m_context.appendConditionalJumpTo(m_context.errorTag());

		m_context << sof::Instruction::BYTE;
		m_context << (u256(1) << (256 - 8)) << sof::Instruction::MUL;
	}
	else if (baseType.category() == Type::Category::TypeType)
	{
		polAssert(baseType.sizeOnStack() == 0, "");
		polAssert(_indexAccess.annotation().type->sizeOnStack() == 0, "");
		// no-op - this seems to be a lone array type (`structType[];`)
	}
	else
		polAssert(false, "Index access only allowed for mappings or arrays.");

	return false;
}

void ExpressionCompiler::endVisit(Identifier const& _identifier)
{
	CompilerContext::LocationSetter locationSetter(m_context, _identifier);
	Declaration const* declaration = _identifier.annotation().referencedDeclaration;
	if (MagicVariableDeclaration const* magicVar = dynamic_cast<MagicVariableDeclaration const*>(declaration))
	{
		switch (magicVar->type()->category())
		{
		case Type::Category::Contract:
			// "this" or "super"
			if (!dynamic_cast<ContractType const&>(*magicVar->type()).isSuper())
				m_context << sof::Instruction::ADDRESS;
			break;
		case Type::Category::Integer:
			// "now"
			m_context << sof::Instruction::TIMESTAMP;
			break;
		default:
			break;
		}
	}
	else if (FunctionDefinition const* functionDef = dynamic_cast<FunctionDefinition const*>(declaration))
		m_context << m_context.virtualFunctionEntryLabel(*functionDef).pushTag();
	else if (auto variable = dynamic_cast<VariableDeclaration const*>(declaration))
	{
		if (!variable->isConstant())
			setLValueFromDeclaration(*declaration, _identifier);
		else
		{
			variable->value()->accept(*this);
			utils().convertType(*variable->value()->annotation().type, *variable->annotation().type);
		}
	}
	else if (auto contract = dynamic_cast<ContractDefinition const*>(declaration))
	{
		if (contract->isLibrary())
			m_context.appendLibraryAddress(contract->name());
	}
	else if (dynamic_cast<EventDefinition const*>(declaration))
	{
		// no-op
	}
	else if (dynamic_cast<EnumDefinition const*>(declaration))
	{
		// no-op
	}
	else if (dynamic_cast<StructDefinition const*>(declaration))
	{
		// no-op
	}
	else
	{
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Identifier type not expected in expression context."));
	}
}

void ExpressionCompiler::endVisit(Literal const& _literal)
{
	CompilerContext::LocationSetter locationSetter(m_context, _literal);
	TypePointer type = _literal.annotation().type;
	
	switch (type->category())
	{
	case Type::Category::IntegerConstant:
	case Type::Category::Bool:
		m_context << type->literalValue(&_literal);
		break;
	case Type::Category::StringLiteral:
		break; // will be done during conversion
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Only integer, boolean and string literals implemented for now."));
	}
}

void ExpressionCompiler::appendAndOrOperatorCode(BinaryOperation const& _binaryOperation)
{
	Token::Value const c_op = _binaryOperation.getOperator();
	polAssert(c_op == Token::Or || c_op == Token::And, "");

	_binaryOperation.leftExpression().accept(*this);
	m_context << sof::Instruction::DUP1;
	if (c_op == Token::And)
		m_context << sof::Instruction::ISZERO;
	sof::AssemblyItem endLabel = m_context.appendConditionalJump();
	m_context << sof::Instruction::POP;
	_binaryOperation.rightExpression().accept(*this);
	m_context << endLabel;
}

void ExpressionCompiler::appendCompareOperatorCode(Token::Value _operator, Type const& _type)
{
	if (_operator == Token::Equal || _operator == Token::NotEqual)
	{
		m_context << sof::Instruction::EQ;
		if (_operator == Token::NotEqual)
			m_context << sof::Instruction::ISZERO;
	}
	else
	{
		bool isSigned = false;
		if (auto type = dynamic_cast<IntegerType const*>(&_type))
			isSigned = type->isSigned();

		switch (_operator)
		{
		case Token::GreaterThanOrEqual:
			m_context <<
				(isSigned ? sof::Instruction::SLT : sof::Instruction::LT) <<
				sof::Instruction::ISZERO;
			break;
		case Token::LessThanOrEqual:
			m_context <<
				(isSigned ? sof::Instruction::SGT : sof::Instruction::GT) <<
				sof::Instruction::ISZERO;
			break;
		case Token::GreaterThan:
			m_context << (isSigned ? sof::Instruction::SGT : sof::Instruction::GT);
			break;
		case Token::LessThan:
			m_context << (isSigned ? sof::Instruction::SLT : sof::Instruction::LT);
			break;
		default:
			BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown comparison operator."));
		}
	}
}

void ExpressionCompiler::appendOrdinaryBinaryOperatorCode(Token::Value _operator, Type const& _type)
{
	if (Token::isArithmeticOp(_operator))
		appendArithmeticOperatorCode(_operator, _type);
	else if (Token::isBitOp(_operator))
		appendBitOperatorCode(_operator);
	else if (Token::isShiftOp(_operator))
		appendShiftOperatorCode(_operator);
	else
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown binary operator."));
}

void ExpressionCompiler::appendArithmeticOperatorCode(Token::Value _operator, Type const& _type)
{
	IntegerType const& type = dynamic_cast<IntegerType const&>(_type);
	bool const c_isSigned = type.isSigned();

	switch (_operator)
	{
	case Token::Add:
		m_context << sof::Instruction::ADD;
		break;
	case Token::Sub:
		m_context << sof::Instruction::SUB;
		break;
	case Token::Mul:
		m_context << sof::Instruction::MUL;
		break;
	case Token::Div:
		m_context  << (c_isSigned ? sof::Instruction::SDIV : sof::Instruction::DIV);
		break;
	case Token::Mod:
		m_context << (c_isSigned ? sof::Instruction::SMOD : sof::Instruction::MOD);
		break;
	case Token::Exp:
		m_context << sof::Instruction::EXP;
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown arithmetic operator."));
	}
}

void ExpressionCompiler::appendBitOperatorCode(Token::Value _operator)
{
	switch (_operator)
	{
	case Token::BitOr:
		m_context << sof::Instruction::OR;
		break;
	case Token::BitAnd:
		m_context << sof::Instruction::AND;
		break;
	case Token::BitXor:
		m_context << sof::Instruction::XOR;
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown bit operator."));
	}
}

void ExpressionCompiler::appendShiftOperatorCode(Token::Value _operator)
{
	BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Shift operators not yet implemented."));
	switch (_operator)
	{
	case Token::SHL:
		break;
	case Token::SAR:
		break;
	default:
		BOOST_THROW_EXCEPTION(InternalCompilerError() << errinfo_comment("Unknown shift operator."));
	}
}

void ExpressionCompiler::appendExternalFunctionCall(
	FunctionType const& _functionType,
	vector<ASTPointer<Expression const>> const& _arguments
)
{
	polAssert(
		_functionType.takesArbitraryParameters() ||
		_arguments.size() == _functionType.parameterTypes().size(), ""
	);

	// Assumed stack content here:
	// <stack top>
	// value [if _functionType.valueSet()]
	// gas [if _functionType.gasSet()]
	// self object [if bound - moved to top right away]
	// function identifier [unless bare]
	// contract address

	unsigned selfSize = _functionType.bound() ? _functionType.selfType()->sizeOnStack() : 0;
	unsigned gasValueSize = (_functionType.gasSet() ? 1 : 0) + (_functionType.valueSet() ? 1 : 0);
	unsigned contractStackPos = m_context.currentToBaseStackOffset(1 + gasValueSize + selfSize + (_functionType.isBareCall() ? 0 : 1));
	unsigned gasStackPos = m_context.currentToBaseStackOffset(gasValueSize);
	unsigned valueStackPos = m_context.currentToBaseStackOffset(1);

	// move self object to top
	if (_functionType.bound())
		utils().moveToStackTop(gasValueSize, _functionType.selfType()->sizeOnStack());

	using FunctionKind = FunctionType::Location;
	FunctionKind funKind = _functionType.location();
	bool returnSuccessCondition = funKind == FunctionKind::Bare || funKind == FunctionKind::BareCallCode;
	bool isCallCode = funKind == FunctionKind::BareCallCode || funKind == FunctionKind::CallCode;
	bool isDelegateCall = funKind == FunctionKind::BareDelegateCall || funKind == FunctionKind::DelegateCall;

	unsigned retSize = 0;
	if (returnSuccessCondition)
		retSize = 0; // return value actually is success condition
	else
		for (auto const& retType: _functionType.returnParameterTypes())
		{
			polAssert(!retType->isDynamicallySized(), "Unable to return dynamic type from external call.");
			retSize += retType->calldataEncodedSize();
		}

	// Evaluate arguments.
	TypePointers argumentTypes;
	TypePointers parameterTypes = _functionType.parameterTypes();
	bool manualFunctionId =
		(funKind == FunctionKind::Bare || funKind == FunctionKind::BareCallCode || funKind == FunctionKind::BareDelegateCall) &&
		!_arguments.empty() &&
		_arguments.front()->annotation().type->mobileType()->calldataEncodedSize(false) ==
			CompilerUtils::dataStartOffset;
	if (manualFunctionId)
	{
		// If we have a Bare* and the first type has exactly 4 bytes, use it as
		// function identifier.
		_arguments.front()->accept(*this);
		utils().convertType(
			*_arguments.front()->annotation().type,
			IntegerType(8 * CompilerUtils::dataStartOffset),
			true
		);
		for (unsigned i = 0; i < gasValueSize; ++i)
			m_context << sof::swapInstruction(gasValueSize - i);
		gasStackPos++;
		valueStackPos++;
	}
	if (_functionType.bound())
	{
		argumentTypes.push_back(_functionType.selfType());
		parameterTypes.insert(parameterTypes.begin(), _functionType.selfType());
	}
	for (size_t i = manualFunctionId ? 1 : 0; i < _arguments.size(); ++i)
	{
		_arguments[i]->accept(*this);
		argumentTypes.push_back(_arguments[i]->annotation().type);
	}

	// Copy function identifier to memory.
	utils().fetchFreeMemoryPointer();
	if (!_functionType.isBareCall() || manualFunctionId)
	{
		m_context << sof::dupInstruction(2 + gasValueSize + CompilerUtils::sizeOnStack(argumentTypes));
		utils().storeInMemoryDynamic(IntegerType(8 * CompilerUtils::dataStartOffset), false);
	}
	// If the function takes arbitrary parameters, copy dynamic length data in place.
	// Move argumenst to memory, will not update the free memory pointer (but will update the memory
	// pointer on the stack).
	utils().encodeToMemory(
		argumentTypes,
		parameterTypes,
		_functionType.padArguments(),
		_functionType.takesArbitraryParameters(),
		isCallCode || isDelegateCall
	);

	// Stack now:
	// <stack top>
	// input_memory_end
	// value [if _functionType.valueSet()]
	// gas [if _functionType.gasSet()]
	// function identifier [unless bare]
	// contract address

	// Output data will replace input data.
	// put on stack: <size of output> <memory pos of output> <size of input> <memory pos of input>
	m_context << u256(retSize);
	utils().fetchFreeMemoryPointer();
	m_context << sof::Instruction::DUP1 << sof::Instruction::DUP4 << sof::Instruction::SUB;
	m_context << sof::Instruction::DUP2;

	// CALL arguments: outSize, outOff, inSize, inOff (already present up to here)
	// [value,] addr, gas (stack top)
	if (isDelegateCall)
		polAssert(!_functionType.valueSet(), "Value set for delegatecall");
	else if (_functionType.valueSet())
		m_context << sof::dupInstruction(m_context.baseToCurrentStackOffset(valueStackPos));
	else
		m_context << u256(0);
	m_context << sof::dupInstruction(m_context.baseToCurrentStackOffset(contractStackPos));

	if (_functionType.gasSet())
		m_context << sof::dupInstruction(m_context.baseToCurrentStackOffset(gasStackPos));
	else
	{
		sof::SVMSchedule schedule;
		// send all gas except the amount needed to execute "SUB" and "CALL"
		// @todo this retains too much gas for now, needs to be fine-tuned.
		u256 gasNeededByCaller = schedule.callGas + 10;
		if (_functionType.valueSet())
			gasNeededByCaller += schedule.callValueTransferGas;
		if (!isCallCode && !isDelegateCall)
			gasNeededByCaller += schedule.callNewAccountGas; // we never know
		m_context <<
			gasNeededByCaller <<
			sof::Instruction::GAS <<
			sof::Instruction::SUB;
	}
	if (isDelegateCall)
		m_context << sof::Instruction::DELEGATECALL;
	else if (isCallCode)
		m_context << sof::Instruction::CALLCODE;
	else
		m_context << sof::Instruction::CALL;

	unsigned remainsSize =
		2 + // contract address, input_memory_end
		_functionType.valueSet() +
		_functionType.gasSet() +
		(!_functionType.isBareCall() || manualFunctionId);

	if (returnSuccessCondition)
		m_context << sof::swapInstruction(remainsSize);
	else
	{
		//Propagate error condition (if CALL pushes 0 on stack).
		m_context << sof::Instruction::ISZERO;
		m_context.appendConditionalJumpTo(m_context.errorTag());
	}

	utils().popStackSlots(remainsSize);

	if (returnSuccessCondition)
	{
		// already there
	}
	else if (funKind == FunctionKind::RIPEMD160)
	{
		// fix: built-in contract returns right-aligned data
		utils().fetchFreeMemoryPointer();
		utils().loadFromMemoryDynamic(IntegerType(160), false, true, false);
		utils().convertType(IntegerType(160), FixedBytesType(20));
	}
	else if (!_functionType.returnParameterTypes().empty())
	{
		utils().fetchFreeMemoryPointer();
		bool memoryNeeded = false;
		for (auto const& retType: _functionType.returnParameterTypes())
		{
			utils().loadFromMemoryDynamic(*retType, false, true, true);
			if (dynamic_cast<ReferenceType const*>(retType.get()))
				memoryNeeded = true;
		}
		if (memoryNeeded)
			utils().storeFreeMemoryPointer();
		else
			m_context << sof::Instruction::POP;
	}
}

void ExpressionCompiler::appendExpressionCopyToMemory(Type const& _expectedType, Expression const& _expression)
{
	polAssert(_expectedType.isValueType(), "Not implemented for non-value types.");
	_expression.accept(*this);
	utils().convertType(*_expression.annotation().type, _expectedType, true);
	utils().storeInMemoryDynamic(_expectedType);
}

void ExpressionCompiler::setLValueFromDeclaration(Declaration const& _declaration, Expression const& _expression)
{
	if (m_context.isLocalVariable(&_declaration))
		setLValue<StackVariable>(_expression, dynamic_cast<VariableDeclaration const&>(_declaration));
	else if (m_context.isStateVariable(&_declaration))
		setLValue<StorageItem>(_expression, dynamic_cast<VariableDeclaration const&>(_declaration));
	else
		BOOST_THROW_EXCEPTION(InternalCompilerError()
			<< errinfo_sourceLocation(_expression.location())
			<< errinfo_comment("Identifier type not supported or identifier not found."));
}

void ExpressionCompiler::setLValueToStorageItem(Expression const& _expression)
{
	setLValue<StorageItem>(_expression, *_expression.annotation().type);
}

CompilerUtils ExpressionCompiler::utils()
{
	return CompilerUtils(m_context);
}

}
}
