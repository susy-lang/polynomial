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
 * @date 2015
 * LValues for use in the expresison compiler.
 */

#include <libpolynomial/LValue.h>
#include <libsvmcore/Instruction.h>
#include <libpolynomial/Types.h>
#include <libpolynomial/AST.h>
#include <libpolynomial/CompilerUtils.h>

using namespace std;
using namespace dev;
using namespace polynomial;


StackVariable::StackVariable(CompilerContext& _compilerContext, Declaration const& _declaration):
	LValue(_compilerContext, *_declaration.type()),
	m_baseStackOffset(m_context.baseStackOffsetOfVariable(_declaration)),
	m_size(m_dataType.sizeOnStack())
{
}

void StackVariable::retrieveValue(SourceLocation const& _location, bool) const
{
	unsigned stackPos = m_context.baseToCurrentStackOffset(m_baseStackOffset);
	if (stackPos + 1 > 16) //@todo correct this by fetching earlier or moving to memory
		BOOST_THROW_EXCEPTION(
			CompilerError() <<
			errinfo_sourceLocation(_location) <<
			errinfo_comment("Stack too deep, try removing local variables.")
		);
	polAssert(stackPos + 1 >= m_size, "Size and stack pos mismatch.");
	for (unsigned i = 0; i < m_size; ++i)
		m_context << sof::dupInstruction(stackPos + 1);
}

void StackVariable::storeValue(Type const&, SourceLocation const& _location, bool _move) const
{
	unsigned stackDiff = m_context.baseToCurrentStackOffset(m_baseStackOffset) - m_size + 1;
	if (stackDiff > 16)
		BOOST_THROW_EXCEPTION(
			CompilerError() <<
			errinfo_sourceLocation(_location) <<
			errinfo_comment("Stack too deep, try removing local variables.")
		);
	else if (stackDiff > 0)
		for (unsigned i = 0; i < m_size; ++i)
			m_context << sof::swapInstruction(stackDiff) << sof::Instruction::POP;
	if (!_move)
		retrieveValue(_location);
}

void StackVariable::setToZero(SourceLocation const& _location, bool) const
{
	CompilerUtils(m_context).pushZeroValue(m_dataType);
	storeValue(m_dataType, _location, true);
}

MemoryItem::MemoryItem(CompilerContext& _compilerContext, Type const& _type, bool _padded):
	LValue(_compilerContext, _type),
	m_padded(_padded)
{
}

void MemoryItem::retrieveValue(SourceLocation const&, bool _remove) const
{
	if (m_dataType.isValueType())
	{
		if (!_remove)
			m_context << sof::Instruction::DUP1;
		CompilerUtils(m_context).loadFromMemoryDynamic(m_dataType, false, m_padded, false);
	}
	else
		m_context << sof::Instruction::MLOAD;
}

void MemoryItem::storeValue(Type const& _sourceType, SourceLocation const&, bool _move) const
{
	CompilerUtils utils(m_context);
	if (m_dataType.isValueType())
	{
		polAssert(_sourceType.isValueType(), "");
		utils.moveIntoStack(_sourceType.sizeOnStack());
		utils.convertType(_sourceType, m_dataType, true);
		if (!_move)
		{
			utils.moveToStackTop(m_dataType.sizeOnStack());
			utils.copyToStackTop(2, m_dataType.sizeOnStack());
		}
		utils.storeInMemoryDynamic(m_dataType, m_padded);
		m_context << sof::Instruction::POP;
	}
	else
	{
		polAssert(_sourceType == m_dataType, "Conversion not implemented for assignment to memory.");

		polAssert(m_dataType.sizeOnStack() == 1, "");
		if (!_move)
			m_context << sof::Instruction::DUP2 << sof::Instruction::SWAP1;
		// stack: [value] value lvalue
		// only store the reference
		m_context << sof::Instruction::MSTORE;
	}
}

void MemoryItem::setToZero(SourceLocation const&, bool _removeReference) const
{
	CompilerUtils utils(m_context);
	if (!_removeReference)
		m_context << sof::Instruction::DUP1;
	utils.pushZeroValue(m_dataType);
	utils.storeInMemoryDynamic(m_dataType, m_padded);
	m_context << sof::Instruction::POP;
}

StorageItem::StorageItem(CompilerContext& _compilerContext, Declaration const& _declaration):
	StorageItem(_compilerContext, *_declaration.type())
{
	auto const& location = m_context.storageLocationOfVariable(_declaration);
	m_context << location.first << u256(location.second);
}

StorageItem::StorageItem(CompilerContext& _compilerContext, Type const& _type):
	LValue(_compilerContext, _type)
{
	if (m_dataType.isValueType())
	{
		polAssert(m_dataType.storageSize() == m_dataType.sizeOnStack(), "");
		polAssert(m_dataType.storageSize() == 1, "Invalid storage size.");
	}
}

void StorageItem::retrieveValue(SourceLocation const&, bool _remove) const
{
	// stack: storage_key storage_offset
	if (!m_dataType.isValueType())
	{
		polAssert(m_dataType.sizeOnStack() == 1, "Invalid storage ref size.");
		if (_remove)
			m_context << sof::Instruction::POP; // remove byte offset
		else
			m_context << sof::Instruction::DUP2;
		return;
	}
	if (!_remove)
		CompilerUtils(m_context).copyToStackTop(sizeOnStack(), sizeOnStack());
	if (m_dataType.storageBytes() == 32)
		m_context << sof::Instruction::POP << sof::Instruction::SLOAD;
	else
	{
		m_context
			<< sof::Instruction::SWAP1 << sof::Instruction::SLOAD << sof::Instruction::SWAP1
			<< u256(0x100) << sof::Instruction::EXP << sof::Instruction::SWAP1 << sof::Instruction::DIV;
		if (m_dataType.category() == Type::Category::FixedBytes)
			m_context << (u256(0x1) << (256 - 8 * m_dataType.storageBytes())) << sof::Instruction::MUL;
		else if (
			m_dataType.category() == Type::Category::Integer &&
			dynamic_cast<IntegerType const&>(m_dataType).isSigned()
		)
			m_context << u256(m_dataType.storageBytes() - 1) << sof::Instruction::SIGNEXTEND;
		else
			m_context << ((u256(0x1) << (8 * m_dataType.storageBytes())) - 1) << sof::Instruction::AND;
	}
}

void StorageItem::storeValue(Type const& _sourceType, SourceLocation const& _location, bool _move) const
{
	CompilerUtils utils(m_context);
	// stack: value storage_key storage_offset
	if (m_dataType.isValueType())
	{
		polAssert(m_dataType.storageBytes() <= 32, "Invalid storage bytes size.");
		polAssert(m_dataType.storageBytes() > 0, "Invalid storage bytes size.");
		if (m_dataType.storageBytes() == 32)
		{
			// offset should be zero
			m_context << sof::Instruction::POP;
			if (!_move)
				m_context << sof::Instruction::DUP2 << sof::Instruction::SWAP1;
			m_context << sof::Instruction::SSTORE;
		}
		else
		{
			// OR the value into the other values in the storage slot
			m_context << u256(0x100) << sof::Instruction::EXP;
			// stack: value storage_ref multiplier
			// fetch old value
			m_context << sof::Instruction::DUP2 << sof::Instruction::SLOAD;
			// stack: value storege_ref multiplier old_full_value
			// clear bytes in old value
			m_context
				<< sof::Instruction::DUP2 << ((u256(1) << (8 * m_dataType.storageBytes())) - 1)
				<< sof::Instruction::MUL;
			m_context << sof::Instruction::NOT << sof::Instruction::AND;
			// stack: value storage_ref multiplier cleared_value
			m_context
				<< sof::Instruction::SWAP1 << sof::Instruction::DUP4;
			// stack: value storage_ref cleared_value multiplier value
			if (m_dataType.category() == Type::Category::FixedBytes)
				m_context
					<< (u256(0x1) << (256 - 8 * dynamic_cast<FixedBytesType const&>(m_dataType).numBytes()))
					<< sof::Instruction::SWAP1 << sof::Instruction::DIV;
			else if (
				m_dataType.category() == Type::Category::Integer &&
				dynamic_cast<IntegerType const&>(m_dataType).isSigned()
			)
				// remove the higher order bits
				m_context
					<< (u256(1) << (8 * (32 - m_dataType.storageBytes())))
					<< sof::Instruction::SWAP1
					<< sof::Instruction::DUP2
					<< sof::Instruction::MUL
					<< sof::Instruction::DIV;
			m_context  << sof::Instruction::MUL << sof::Instruction::OR;
			// stack: value storage_ref updated_value
			m_context << sof::Instruction::SWAP1 << sof::Instruction::SSTORE;
			if (_move)
				m_context << sof::Instruction::POP;
		}
	}
	else
	{
		polAssert(
			_sourceType.category() == m_dataType.category(),
			"Wrong type conversation for assignment.");
		if (m_dataType.category() == Type::Category::Array)
		{
			m_context << sof::Instruction::POP; // remove byte offset
			ArrayUtils(m_context).copyArrayToStorage(
						dynamic_cast<ArrayType const&>(m_dataType),
						dynamic_cast<ArrayType const&>(_sourceType));
			if (_move)
				m_context << sof::Instruction::POP;
		}
		else if (m_dataType.category() == Type::Category::Struct)
		{
			// stack layout: source_ref target_ref target_offset
			// note that we have structs, so offset should be zero and are ignored
			m_context << sof::Instruction::POP;
			auto const& structType = dynamic_cast<StructType const&>(m_dataType);
			auto const& sourceType = dynamic_cast<StructType const&>(_sourceType);
			polAssert(
				structType.structDefinition() == sourceType.structDefinition(),
				"Struct assignment with conversion."
			);
			polAssert(sourceType.location() != DataLocation::CallData, "Structs in calldata not supported.");
			for (auto const& member: structType.members())
			{
				// assign each member that is not a mapping
				TypePointer const& memberType = member.type;
				if (memberType->category() == Type::Category::Mapping)
					continue;
				TypePointer sourceMemberType = sourceType.memberType(member.name);
				if (sourceType.location() == DataLocation::Storage)
				{
					// stack layout: source_ref target_ref
					pair<u256, unsigned> const& offsets = sourceType.storageOffsetsOfMember(member.name);
					m_context << offsets.first << sof::Instruction::DUP3 << sof::Instruction::ADD;
					m_context << u256(offsets.second);
					// stack: source_ref target_ref source_member_ref source_member_off
					StorageItem(m_context, *sourceMemberType).retrieveValue(_location, true);
					// stack: source_ref target_ref source_value...
				}
				else
				{
					polAssert(sourceType.location() == DataLocation::Memory, "");
					// stack layout: source_ref target_ref
					TypePointer sourceMemberType = sourceType.memberType(member.name);
					m_context << sourceType.memoryOffsetOfMember(member.name);
					m_context << sof::Instruction::DUP3 << sof::Instruction::ADD;
					MemoryItem(m_context, *sourceMemberType).retrieveValue(_location, true);
					// stack layout: source_ref target_ref source_value...
				}
				unsigned stackSize = sourceMemberType->sizeOnStack();
				pair<u256, unsigned> const& offsets = structType.storageOffsetsOfMember(member.name);
				m_context << sof::dupInstruction(1 + stackSize) << offsets.first << sof::Instruction::ADD;
				m_context << u256(offsets.second);
				// stack: source_ref target_ref target_off source_value... target_member_ref target_member_byte_off
				StorageItem(m_context, *memberType).storeValue(*sourceMemberType, _location, true);
			}
			// stack layout: source_ref target_ref
			polAssert(sourceType.sizeOnStack() == 1, "Unexpected source size.");
			if (_move)
				utils.popStackSlots(2);
			else
				m_context << sof::Instruction::SWAP1 << sof::Instruction::POP;
		}
		else
			BOOST_THROW_EXCEPTION(
				InternalCompilerError()
					<< errinfo_sourceLocation(_location)
					<< errinfo_comment("Invalid non-value type for assignment."));
	}
}

void StorageItem::setToZero(SourceLocation const&, bool _removeReference) const
{
	if (m_dataType.category() == Type::Category::Array)
	{
		if (!_removeReference)
			CompilerUtils(m_context).copyToStackTop(sizeOnStack(), sizeOnStack());
		ArrayUtils(m_context).clearArray(dynamic_cast<ArrayType const&>(m_dataType));
	}
	else if (m_dataType.category() == Type::Category::Struct)
	{
		// stack layout: storage_key storage_offset
		// @todo this can be improved: use StorageItem for non-value types, and just store 0 in
		// all slots that contain value types later.
		auto const& structType = dynamic_cast<StructType const&>(m_dataType);
		for (auto const& member: structType.members())
		{
			// zero each member that is not a mapping
			TypePointer const& memberType = member.type;
			if (memberType->category() == Type::Category::Mapping)
				continue;
			pair<u256, unsigned> const& offsets = structType.storageOffsetsOfMember(member.name);
			m_context
				<< offsets.first << sof::Instruction::DUP3 << sof::Instruction::ADD
				<< u256(offsets.second);
			StorageItem(m_context, *memberType).setToZero();
		}
		if (_removeReference)
			m_context << sof::Instruction::POP << sof::Instruction::POP;
	}
	else
	{
		polAssert(m_dataType.isValueType(), "Clearing of unsupported type requested: " + m_dataType.toString());
		if (!_removeReference)
			CompilerUtils(m_context).copyToStackTop(sizeOnStack(), sizeOnStack());
		if (m_dataType.storageBytes() == 32)
		{
			// offset should be zero
			m_context
				<< sof::Instruction::POP << u256(0)
				<< sof::Instruction::SWAP1 << sof::Instruction::SSTORE;
		}
		else
		{
			m_context << u256(0x100) << sof::Instruction::EXP;
			// stack: storage_ref multiplier
			// fetch old value
			m_context << sof::Instruction::DUP2 << sof::Instruction::SLOAD;
			// stack: storege_ref multiplier old_full_value
			// clear bytes in old value
			m_context
				<< sof::Instruction::SWAP1 << ((u256(1) << (8 * m_dataType.storageBytes())) - 1)
				<< sof::Instruction::MUL;
			m_context << sof::Instruction::NOT << sof::Instruction::AND;
			// stack: storage_ref cleared_value
			m_context << sof::Instruction::SWAP1 << sof::Instruction::SSTORE;
		}
	}
}

/// Used in StorageByteArrayElement
static FixedBytesType byteType(1);

StorageByteArrayElement::StorageByteArrayElement(CompilerContext& _compilerContext):
	LValue(_compilerContext, byteType)
{
}

void StorageByteArrayElement::retrieveValue(SourceLocation const&, bool _remove) const
{
	// stack: ref byte_number
	if (_remove)
		m_context << sof::Instruction::SWAP1 << sof::Instruction::SLOAD
			<< sof::Instruction::SWAP1 << sof::Instruction::BYTE;
	else
		m_context << sof::Instruction::DUP2 << sof::Instruction::SLOAD
			<< sof::Instruction::DUP2 << sof::Instruction::BYTE;
	m_context << (u256(1) << (256 - 8)) << sof::Instruction::MUL;
}

void StorageByteArrayElement::storeValue(Type const&, SourceLocation const&, bool _move) const
{
	// stack: value ref byte_number
	m_context << u256(31) << sof::Instruction::SUB << u256(0x100) << sof::Instruction::EXP;
	// stack: value ref (1<<(8*(31-byte_number)))
	m_context << sof::Instruction::DUP2 << sof::Instruction::SLOAD;
	// stack: value ref (1<<(8*(31-byte_number))) old_full_value
	// clear byte in old value
	m_context << sof::Instruction::DUP2 << u256(0xff) << sof::Instruction::MUL
		<< sof::Instruction::NOT << sof::Instruction::AND;
	// stack: value ref (1<<(32-byte_number)) old_full_value_with_cleared_byte
	m_context << sof::Instruction::SWAP1;
	m_context << (u256(1) << (256 - 8)) << sof::Instruction::DUP5 << sof::Instruction::DIV
		<< sof::Instruction::MUL << sof::Instruction::OR;
	// stack: value ref new_full_value
	m_context << sof::Instruction::SWAP1 << sof::Instruction::SSTORE;
	if (_move)
		m_context << sof::Instruction::POP;
}

void StorageByteArrayElement::setToZero(SourceLocation const&, bool _removeReference) const
{
	// stack: ref byte_number
	if (!_removeReference)
		m_context << sof::Instruction::DUP2 << sof::Instruction::DUP2;
	m_context << u256(31) << sof::Instruction::SUB << u256(0x100) << sof::Instruction::EXP;
	// stack: ref (1<<(8*(31-byte_number)))
	m_context << sof::Instruction::DUP2 << sof::Instruction::SLOAD;
	// stack: ref (1<<(8*(31-byte_number))) old_full_value
	// clear byte in old value
	m_context << sof::Instruction::SWAP1 << u256(0xff) << sof::Instruction::MUL;
	m_context << sof::Instruction::NOT << sof::Instruction::AND;
	// stack: ref old_full_value_with_cleared_byte
	m_context << sof::Instruction::SWAP1 << sof::Instruction::SSTORE;
}

StorageArrayLength::StorageArrayLength(CompilerContext& _compilerContext, const ArrayType& _arrayType):
	LValue(_compilerContext, *_arrayType.memberType("length")),
	m_arrayType(_arrayType)
{
	polAssert(m_arrayType.isDynamicallySized(), "");
}

void StorageArrayLength::retrieveValue(SourceLocation const&, bool _remove) const
{
	if (!_remove)
		m_context << sof::Instruction::DUP1;
	m_context << sof::Instruction::SLOAD;
}

void StorageArrayLength::storeValue(Type const&, SourceLocation const&, bool _move) const
{
	if (_move)
		m_context << sof::Instruction::SWAP1;
	else
		m_context << sof::Instruction::DUP2;
	ArrayUtils(m_context).resizeDynamicArray(m_arrayType);
}

void StorageArrayLength::setToZero(SourceLocation const&, bool _removeReference) const
{
	if (!_removeReference)
		m_context << sof::Instruction::DUP1;
	ArrayUtils(m_context).clearDynamicArray(m_arrayType);
}
