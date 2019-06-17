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
 * Code generation utils that handle arrays.
 */

#include <libpolynomial/codegen/ArrayUtils.h>
#include <libsvmcore/Instruction.h>
#include <libpolynomial/codegen/CompilerContext.h>
#include <libpolynomial/codegen/CompilerUtils.h>
#include <libpolynomial/ast/Types.h>
#include <libpolynomial/interface/Utils.h>
#include <libpolynomial/codegen/LValue.h>

using namespace std;
using namespace dev;
using namespace polynomial;

void ArrayUtils::copyArrayToStorage(ArrayType const& _targetType, ArrayType const& _sourceType) const
{
	// this copies source to target and also clears target if it was larger
	// need to leave "target_ref target_byte_off" on the stack at the end

	// stack layout: [source_ref] [source length] target_ref (top)
	polAssert(_targetType.location() == DataLocation::Storage, "");

	IntegerType uint256(256);
	Type const* targetBaseType = _targetType.isByteArray() ? &uint256 : &(*_targetType.baseType());
	Type const* sourceBaseType = _sourceType.isByteArray() ? &uint256 : &(*_sourceType.baseType());

	// TODO unroll loop for small sizes

	bool sourceIsStorage = _sourceType.location() == DataLocation::Storage;
	bool fromCalldata = _sourceType.location() == DataLocation::CallData;
	bool directCopy = sourceIsStorage && sourceBaseType->isValueType() && *sourceBaseType == *targetBaseType;
	bool haveByteOffsetSource = !directCopy && sourceIsStorage && sourceBaseType->storageBytes() <= 16;
	bool haveByteOffsetTarget = !directCopy && targetBaseType->storageBytes() <= 16;
	unsigned byteOffsetSize = (haveByteOffsetSource ? 1 : 0) + (haveByteOffsetTarget ? 1 : 0);

	// stack: source_ref [source_length] target_ref
	// store target_ref
	for (unsigned i = _sourceType.sizeOnStack(); i > 0; --i)
		m_context << sof::swapInstruction(i);
	// stack: target_ref source_ref [source_length]
	// stack: target_ref source_ref [source_length]
	// retrieve source length
	if (_sourceType.location() != DataLocation::CallData || !_sourceType.isDynamicallySized())
		retrieveLength(_sourceType); // otherwise, length is already there
	if (_sourceType.location() == DataLocation::Memory && _sourceType.isDynamicallySized())
	{
		// increment source pointer to point to data
		m_context << sof::Instruction::SWAP1 << u256(0x20);
		m_context << sof::Instruction::ADD << sof::Instruction::SWAP1;
	}

	// stack: target_ref source_ref source_length
	m_context << sof::Instruction::DUP3;
	// stack: target_ref source_ref source_length target_ref
	retrieveLength(_targetType);
	// stack: target_ref source_ref source_length target_ref target_length
	if (_targetType.isDynamicallySized())
		// store new target length
		if (!_targetType.isByteArray())
			// Otherwise, length will be stored below.
			m_context << sof::Instruction::DUP3 << sof::Instruction::DUP3 << sof::Instruction::SSTORE;
	if (sourceBaseType->category() == Type::Category::Mapping)
	{
		polAssert(targetBaseType->category() == Type::Category::Mapping, "");
		polAssert(_sourceType.location() == DataLocation::Storage, "");
		// nothing to copy
		m_context
			<< sof::Instruction::POP << sof::Instruction::POP
			<< sof::Instruction::POP << sof::Instruction::POP;
		return;
	}
	// stack: target_ref source_ref source_length target_ref target_length
	// compute hashes (data positions)
	m_context << sof::Instruction::SWAP1;
	if (_targetType.isDynamicallySized())
		CompilerUtils(m_context).computeHashStatic();
	// stack: target_ref source_ref source_length target_length target_data_pos
	m_context << sof::Instruction::SWAP1;
	convertLengthToSize(_targetType);
	m_context << sof::Instruction::DUP2 << sof::Instruction::ADD;
	// stack: target_ref source_ref source_length target_data_pos target_data_end
	m_context << sof::Instruction::SWAP3;
	// stack: target_ref target_data_end source_length target_data_pos source_ref

	sof::AssemblyItem copyLoopEndWithoutByteOffset = m_context.newTag();

	// special case for short byte arrays: Store them together with their length.
	if (_targetType.isByteArray())
	{
		// stack: target_ref target_data_end source_length target_data_pos source_ref
		m_context << sof::Instruction::DUP3 << u256(31) << sof::Instruction::LT;
		sof::AssemblyItem longByteArray = m_context.appendConditionalJump();
		// store the short byte array
		polAssert(_sourceType.isByteArray(), "");
		if (_sourceType.location() == DataLocation::Storage)
		{
			// just copy the slot, it contains length and data
			m_context << sof::Instruction::DUP1 << sof::Instruction::SLOAD;
			m_context << sof::Instruction::DUP6 << sof::Instruction::SSTORE;
		}
		else
		{
			m_context << sof::Instruction::DUP1;
			CompilerUtils(m_context).loadFromMemoryDynamic(*sourceBaseType, fromCalldata, true, false);
			// stack: target_ref target_data_end source_length target_data_pos source_ref value
			// clear the lower-order byte - which will hold the length
			m_context << u256(0xff) << sof::Instruction::NOT << sof::Instruction::AND;
			// fetch the length and shift it left by one
			m_context << sof::Instruction::DUP4 << sof::Instruction::DUP1 << sof::Instruction::ADD;
			// combine value and length and store them
			m_context << sof::Instruction::OR << sof::Instruction::DUP6 << sof::Instruction::SSTORE;
		}
		// end of special case, jump right into cleaning target data area
		m_context.appendJumpTo(copyLoopEndWithoutByteOffset);
		m_context << longByteArray;
		// Store length (2*length+1)
		m_context << sof::Instruction::DUP3 << sof::Instruction::DUP1 << sof::Instruction::ADD;
		m_context << u256(1) << sof::Instruction::ADD;
		m_context << sof::Instruction::DUP6 << sof::Instruction::SSTORE;
	}

	// skip copying if source length is zero
	m_context << sof::Instruction::DUP3 << sof::Instruction::ISZERO;
	m_context.appendConditionalJumpTo(copyLoopEndWithoutByteOffset);

	if (_sourceType.location() == DataLocation::Storage && _sourceType.isDynamicallySized())
		CompilerUtils(m_context).computeHashStatic();
	// stack: target_ref target_data_end source_length target_data_pos source_data_pos
	m_context << sof::Instruction::SWAP2;
	convertLengthToSize(_sourceType);
	m_context << sof::Instruction::DUP3 << sof::Instruction::ADD;
	// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end
	if (haveByteOffsetTarget)
		m_context << u256(0);
	if (haveByteOffsetSource)
		m_context << u256(0);
	// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end [target_byte_offset] [source_byte_offset]
	sof::AssemblyItem copyLoopStart = m_context.newTag();
	m_context << copyLoopStart;
	// check for loop condition
	m_context
		<< sof::dupInstruction(3 + byteOffsetSize) << sof::dupInstruction(2 + byteOffsetSize)
		<< sof::Instruction::GT << sof::Instruction::ISZERO;
	sof::AssemblyItem copyLoopEnd = m_context.appendConditionalJump();
	// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end [target_byte_offset] [source_byte_offset]
	// copy
	if (sourceBaseType->category() == Type::Category::Array)
	{
		polAssert(byteOffsetSize == 0, "Byte offset for array as base type.");
		auto const& sourceBaseArrayType = dynamic_cast<ArrayType const&>(*sourceBaseType);
		m_context << sof::Instruction::DUP3;
		if (sourceBaseArrayType.location() == DataLocation::Memory)
			m_context << sof::Instruction::MLOAD;
		m_context << sof::Instruction::DUP3;
		copyArrayToStorage(dynamic_cast<ArrayType const&>(*targetBaseType), sourceBaseArrayType);
		m_context << sof::Instruction::POP;
	}
	else if (directCopy)
	{
		polAssert(byteOffsetSize == 0, "Byte offset for direct copy.");
		m_context
			<< sof::Instruction::DUP3 << sof::Instruction::SLOAD
			<< sof::Instruction::DUP3 << sof::Instruction::SSTORE;
	}
	else
	{
		// Note that we have to copy each element on its own in case conversion is involved.
		// We might copy too much if there is padding at the last element, but this way end
		// checking is easier.
		// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end [target_byte_offset] [source_byte_offset]
		m_context << sof::dupInstruction(3 + byteOffsetSize);
		if (_sourceType.location() == DataLocation::Storage)
		{
			if (haveByteOffsetSource)
				m_context << sof::Instruction::DUP2;
			else
				m_context << u256(0);
			StorageItem(m_context, *sourceBaseType).retrieveValue(SourceLocation(), true);
		}
		else if (sourceBaseType->isValueType())
			CompilerUtils(m_context).loadFromMemoryDynamic(*sourceBaseType, fromCalldata, true, false);
		else
			polAssert(false, "Copying of type " + _sourceType.toString(false) + " to storage not yet supported.");
		// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end [target_byte_offset] [source_byte_offset] <source_value>...
		polAssert(
			2 + byteOffsetSize + sourceBaseType->sizeOnStack() <= 16,
			"Stack too deep, try removing local variables."
		);
		// fetch target storage reference
		m_context << sof::dupInstruction(2 + byteOffsetSize + sourceBaseType->sizeOnStack());
		if (haveByteOffsetTarget)
			m_context << sof::dupInstruction(1 + byteOffsetSize + sourceBaseType->sizeOnStack());
		else
			m_context << u256(0);
		StorageItem(m_context, *targetBaseType).storeValue(*sourceBaseType, SourceLocation(), true);
	}
	// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end [target_byte_offset] [source_byte_offset]
	// increment source
	if (haveByteOffsetSource)
		incrementByteOffset(sourceBaseType->storageBytes(), 1, haveByteOffsetTarget ? 5 : 4);
	else
	{
		m_context << sof::swapInstruction(2 + byteOffsetSize);
		if (sourceIsStorage)
			m_context << sourceBaseType->storageSize();
		else if (_sourceType.location() == DataLocation::Memory)
			m_context << sourceBaseType->memoryHeadSize();
		else
			m_context << sourceBaseType->calldataEncodedSize(true);
		m_context
			<< sof::Instruction::ADD
			<< sof::swapInstruction(2 + byteOffsetSize);
	}
	// increment target
	if (haveByteOffsetTarget)
		incrementByteOffset(targetBaseType->storageBytes(), byteOffsetSize, byteOffsetSize + 2);
	else
		m_context
			<< sof::swapInstruction(1 + byteOffsetSize)
			<< targetBaseType->storageSize()
			<< sof::Instruction::ADD
			<< sof::swapInstruction(1 + byteOffsetSize);
	m_context.appendJumpTo(copyLoopStart);
	m_context << copyLoopEnd;
	if (haveByteOffsetTarget)
	{
		// clear elements that might be left over in the current slot in target
		// stack: target_ref target_data_end source_data_pos target_data_pos source_data_end target_byte_offset [source_byte_offset]
		m_context << sof::dupInstruction(byteOffsetSize) << sof::Instruction::ISZERO;
		sof::AssemblyItem copyCleanupLoopEnd = m_context.appendConditionalJump();
		m_context << sof::dupInstruction(2 + byteOffsetSize) << sof::dupInstruction(1 + byteOffsetSize);
		StorageItem(m_context, *targetBaseType).setToZero(SourceLocation(), true);
		incrementByteOffset(targetBaseType->storageBytes(), byteOffsetSize, byteOffsetSize + 2);
		m_context.appendJumpTo(copyLoopEnd);

		m_context << copyCleanupLoopEnd;
		m_context << sof::Instruction::POP; // might pop the source, but then target is popped next
	}
	if (haveByteOffsetSource)
		m_context << sof::Instruction::POP;
	m_context << copyLoopEndWithoutByteOffset;

	// zero-out leftovers in target
	// stack: target_ref target_data_end source_data_pos target_data_pos_updated source_data_end
	m_context << sof::Instruction::POP << sof::Instruction::SWAP1 << sof::Instruction::POP;
	// stack: target_ref target_data_end target_data_pos_updated
	clearStorageLoop(*targetBaseType);
	m_context << sof::Instruction::POP;
}

void ArrayUtils::copyArrayToMemory(ArrayType const& _sourceType, bool _padToWordBoundaries) const
{
	polAssert(
		!_sourceType.baseType()->isDynamicallySized(),
		"Nested dynamic arrays not implemented here."
	);
	CompilerUtils utils(m_context);
	unsigned baseSize = 1;
	if (!_sourceType.isByteArray())
		// We always pad the elements, regardless of _padToWordBoundaries.
		baseSize = _sourceType.baseType()->calldataEncodedSize();

	if (_sourceType.location() == DataLocation::CallData)
	{
		if (!_sourceType.isDynamicallySized())
			m_context << _sourceType.length();
		if (baseSize > 1)
			m_context << u256(baseSize) << sof::Instruction::MUL;
		// stack: target source_offset source_len
		m_context << sof::Instruction::DUP1 << sof::Instruction::DUP3 << sof::Instruction::DUP5;
		// stack: target source_offset source_len source_len source_offset target
		m_context << sof::Instruction::CALLDATACOPY;
		m_context << sof::Instruction::DUP3 << sof::Instruction::ADD;
		m_context << sof::Instruction::SWAP2 << sof::Instruction::POP << sof::Instruction::POP;
	}
	else if (_sourceType.location() == DataLocation::Memory)
	{
		retrieveLength(_sourceType);
		// stack: target source length
		if (!_sourceType.baseType()->isValueType())
		{
			// copy using a loop
			m_context << u256(0) << sof::Instruction::SWAP3;
			// stack: counter source length target
			auto repeat = m_context.newTag();
			m_context << repeat;
			m_context << sof::Instruction::DUP2 << sof::Instruction::DUP5;
			m_context << sof::Instruction::LT << sof::Instruction::ISZERO;
			auto loopEnd = m_context.appendConditionalJump();
			m_context << sof::Instruction::DUP3 << sof::Instruction::DUP5;
			accessIndex(_sourceType, false);
			MemoryItem(m_context, *_sourceType.baseType(), true).retrieveValue(SourceLocation(), true);
			if (auto baseArray = dynamic_cast<ArrayType const*>(_sourceType.baseType().get()))
				copyArrayToMemory(*baseArray, _padToWordBoundaries);
			else
				utils.storeInMemoryDynamic(*_sourceType.baseType());
			m_context << sof::Instruction::SWAP3 << u256(1) << sof::Instruction::ADD;
			m_context << sof::Instruction::SWAP3;
			m_context.appendJumpTo(repeat);
			m_context << loopEnd;
			m_context << sof::Instruction::SWAP3;
			utils.popStackSlots(3);
			// stack: updated_target_pos
			return;
		}

		// memcpy using the built-in contract
		if (_sourceType.isDynamicallySized())
		{
			// change pointer to data part
			m_context << sof::Instruction::SWAP1 << u256(32) << sof::Instruction::ADD;
			m_context << sof::Instruction::SWAP1;
		}
		// convert length to size
		if (baseSize > 1)
			m_context << u256(baseSize) << sof::Instruction::MUL;
		// stack: <target> <source> <size>
		//@TODO do not use ::CALL if less than 32 bytes?
		m_context << sof::Instruction::DUP1 << sof::Instruction::DUP4 << sof::Instruction::DUP4;
		utils.memoryCopy();

		m_context << sof::Instruction::SWAP1 << sof::Instruction::POP;
		// stack: <target> <size>

		bool paddingNeeded = false;
		if (_sourceType.isDynamicallySized())
			paddingNeeded = _padToWordBoundaries && ((baseSize % 32) != 0);
		else
			paddingNeeded = _padToWordBoundaries && (((_sourceType.length() * baseSize) % 32) != 0);
		if (paddingNeeded)
		{
			// stack: <target> <size>
			m_context << sof::Instruction::SWAP1 << sof::Instruction::DUP2 << sof::Instruction::ADD;
			// stack: <length> <target + size>
			m_context << sof::Instruction::SWAP1 << u256(31) << sof::Instruction::AND;
			// stack: <target + size> <remainder = size % 32>
			sof::AssemblyItem skip = m_context.newTag();
			if (_sourceType.isDynamicallySized())
			{
				m_context << sof::Instruction::DUP1 << sof::Instruction::ISZERO;
				m_context.appendConditionalJumpTo(skip);
			}
			// round off, load from there.
			// stack <target + size> <remainder = size % 32>
			m_context << sof::Instruction::DUP1 << sof::Instruction::DUP3;
			m_context << sof::Instruction::SUB;
			// stack: target+size remainder <target + size - remainder>
			m_context << sof::Instruction::DUP1 << sof::Instruction::MLOAD;
			// Now we AND it with ~(2**(8 * (32 - remainder)) - 1)
			m_context << u256(1);
			m_context << sof::Instruction::DUP4 << u256(32) << sof::Instruction::SUB;
			// stack: ...<v> 1 <32 - remainder>
			m_context << u256(0x100) << sof::Instruction::EXP << sof::Instruction::SUB;
			m_context << sof::Instruction::NOT << sof::Instruction::AND;
			// stack: target+size remainder target+size-remainder <v & ...>
			m_context << sof::Instruction::DUP2 << sof::Instruction::MSTORE;
			// stack: target+size remainder target+size-remainder
			m_context << u256(32) << sof::Instruction::ADD;
			// stack: target+size remainder <new_padded_end>
			m_context << sof::Instruction::SWAP2 << sof::Instruction::POP;

			if (_sourceType.isDynamicallySized())
				m_context << skip.tag();
			// stack <target + "size"> <remainder = size % 32>
			m_context << sof::Instruction::POP;
		}
		else
			// stack: <target> <size>
			m_context << sof::Instruction::ADD;
	}
	else
	{
		polAssert(_sourceType.location() == DataLocation::Storage, "");
		unsigned storageBytes = _sourceType.baseType()->storageBytes();
		u256 storageSize = _sourceType.baseType()->storageSize();
		polAssert(storageSize > 1 || (storageSize == 1 && storageBytes > 0), "");

		retrieveLength(_sourceType);
		// stack here: memory_offset storage_offset length
		// jump to end if length is zero
		m_context << sof::Instruction::DUP1 << sof::Instruction::ISZERO;
		sof::AssemblyItem loopEnd = m_context.appendConditionalJump();
		// Special case for tightly-stored byte arrays
		if (_sourceType.isByteArray())
		{
			// stack here: memory_offset storage_offset length
			m_context << sof::Instruction::DUP1 << u256(31) << sof::Instruction::LT;
			sof::AssemblyItem longByteArray = m_context.appendConditionalJump();
			// store the short byte array (discard lower-order byte)
			m_context << u256(0x100) << sof::Instruction::DUP1;
			m_context << sof::Instruction::DUP4 << sof::Instruction::SLOAD;
			m_context << sof::Instruction::DIV << sof::Instruction::MUL;
			m_context << sof::Instruction::DUP4 << sof::Instruction::MSTORE;
			// stack here: memory_offset storage_offset length
			// add 32 or length to memory offset
			m_context << sof::Instruction::SWAP2;
			if (_padToWordBoundaries)
				m_context << u256(32);
			else
				m_context << sof::Instruction::DUP3;
			m_context << sof::Instruction::ADD;
			m_context << sof::Instruction::SWAP2;
			m_context.appendJumpTo(loopEnd);
			m_context << longByteArray;
		}
		// compute memory end offset
		if (baseSize > 1)
			// convert length to memory size
			m_context << u256(baseSize) << sof::Instruction::MUL;
		m_context << sof::Instruction::DUP3 << sof::Instruction::ADD << sof::Instruction::SWAP2;
		if (_sourceType.isDynamicallySized())
		{
			// actual array data is stored at SHA3(storage_offset)
			m_context << sof::Instruction::SWAP1;
			utils.computeHashStatic();
			m_context << sof::Instruction::SWAP1;
		}

		// stack here: memory_end_offset storage_data_offset memory_offset
		bool haveByteOffset = !_sourceType.isByteArray() && storageBytes <= 16;
		if (haveByteOffset)
			m_context << u256(0) << sof::Instruction::SWAP1;
		// stack here: memory_end_offset storage_data_offset [storage_byte_offset] memory_offset
		sof::AssemblyItem loopStart = m_context.newTag();
		m_context << loopStart;
		// load and store
		if (_sourceType.isByteArray())
		{
			// Packed both in storage and memory.
			m_context << sof::Instruction::DUP2 << sof::Instruction::SLOAD;
			m_context << sof::Instruction::DUP2 << sof::Instruction::MSTORE;
			// increment storage_data_offset by 1
			m_context << sof::Instruction::SWAP1 << u256(1) << sof::Instruction::ADD;
			// increment memory offset by 32
			m_context << sof::Instruction::SWAP1 << u256(32) << sof::Instruction::ADD;
		}
		else
		{
			// stack here: memory_end_offset storage_data_offset [storage_byte_offset] memory_offset
			if (haveByteOffset)
				m_context << sof::Instruction::DUP3 << sof::Instruction::DUP3;
			else
				m_context << sof::Instruction::DUP2 << u256(0);
			StorageItem(m_context, *_sourceType.baseType()).retrieveValue(SourceLocation(), true);
			if (auto baseArray = dynamic_cast<ArrayType const*>(_sourceType.baseType().get()))
				copyArrayToMemory(*baseArray, _padToWordBoundaries);
			else
				utils.storeInMemoryDynamic(*_sourceType.baseType());
			// increment storage_data_offset and byte offset
			if (haveByteOffset)
				incrementByteOffset(storageBytes, 2, 3);
			else
			{
				m_context << sof::Instruction::SWAP1;
				m_context << storageSize << sof::Instruction::ADD;
				m_context << sof::Instruction::SWAP1;
			}
		}
		// check for loop condition
		m_context << sof::Instruction::DUP1 << sof::dupInstruction(haveByteOffset ? 5 : 4);
		m_context << sof::Instruction::GT;
		m_context.appendConditionalJumpTo(loopStart);
		// stack here: memory_end_offset storage_data_offset [storage_byte_offset] memory_offset
		if (haveByteOffset)
			m_context << sof::Instruction::SWAP1 << sof::Instruction::POP;
		if (_padToWordBoundaries && baseSize % 32 != 0)
		{
			// memory_end_offset - start is the actual length (we want to compute the ceil of).
			// memory_offset - start is its next multiple of 32, but it might be off by 32.
			// so we compute: memory_end_offset += (memory_offset - memory_end_offest) & 31
			m_context << sof::Instruction::DUP3 << sof::Instruction::SWAP1 << sof::Instruction::SUB;
			m_context << u256(31) << sof::Instruction::AND;
			m_context << sof::Instruction::DUP3 << sof::Instruction::ADD;
			m_context << sof::Instruction::SWAP2;
		}
		m_context << loopEnd << sof::Instruction::POP << sof::Instruction::POP;
	}
}

void ArrayUtils::clearArray(ArrayType const& _type) const
{
	unsigned stackHeightStart = m_context.stackHeight();
	polAssert(_type.location() == DataLocation::Storage, "");
	if (_type.baseType()->storageBytes() < 32)
	{
		polAssert(_type.baseType()->isValueType(), "Invalid storage size for non-value type.");
		polAssert(_type.baseType()->storageSize() <= 1, "Invalid storage size for type.");
	}
	if (_type.baseType()->isValueType())
		polAssert(_type.baseType()->storageSize() <= 1, "Invalid size for value type.");

	m_context << sof::Instruction::POP; // remove byte offset
	if (_type.isDynamicallySized())
		clearDynamicArray(_type);
	else if (_type.length() == 0 || _type.baseType()->category() == Type::Category::Mapping)
		m_context << sof::Instruction::POP;
	else if (_type.baseType()->isValueType() && _type.storageSize() <= 5)
	{
		// unroll loop for small arrays @todo choose a good value
		// Note that we loop over storage slots here, not elements.
		for (unsigned i = 1; i < _type.storageSize(); ++i)
			m_context
				<< u256(0) << sof::Instruction::DUP2 << sof::Instruction::SSTORE
				<< u256(1) << sof::Instruction::ADD;
		m_context << u256(0) << sof::Instruction::SWAP1 << sof::Instruction::SSTORE;
	}
	else if (!_type.baseType()->isValueType() && _type.length() <= 4)
	{
		// unroll loop for small arrays @todo choose a good value
		polAssert(_type.baseType()->storageBytes() >= 32, "Invalid storage size.");
		for (unsigned i = 1; i < _type.length(); ++i)
		{
			m_context << u256(0);
			StorageItem(m_context, *_type.baseType()).setToZero(SourceLocation(), false);
			m_context
				<< sof::Instruction::POP
				<< u256(_type.baseType()->storageSize()) << sof::Instruction::ADD;
		}
		m_context << u256(0);
		StorageItem(m_context, *_type.baseType()).setToZero(SourceLocation(), true);
	}
	else
	{
		m_context << sof::Instruction::DUP1 << _type.length();
		convertLengthToSize(_type);
		m_context << sof::Instruction::ADD << sof::Instruction::SWAP1;
		if (_type.baseType()->storageBytes() < 32)
			clearStorageLoop(IntegerType(256));
		else
			clearStorageLoop(*_type.baseType());
		m_context << sof::Instruction::POP;
	}
	polAssert(m_context.stackHeight() == stackHeightStart - 2, "");
}

void ArrayUtils::clearDynamicArray(ArrayType const& _type) const
{
	polAssert(_type.location() == DataLocation::Storage, "");
	polAssert(_type.isDynamicallySized(), "");

	// fetch length
	retrieveLength(_type);
	// set length to zero
	m_context << u256(0) << sof::Instruction::DUP3 << sof::Instruction::SSTORE;
	// Special case: short byte arrays are stored togeher with their length
	sof::AssemblyItem endTag = m_context.newTag();
	if (_type.isByteArray())
	{
		// stack: ref old_length
		m_context << sof::Instruction::DUP1 << u256(31) << sof::Instruction::LT;
		sof::AssemblyItem longByteArray = m_context.appendConditionalJump();
		m_context << sof::Instruction::POP;
		m_context.appendJumpTo(endTag);
		m_context.adjustStackOffset(1); // needed because of jump
		m_context << longByteArray;
	}
	// stack: ref old_length
	convertLengthToSize(_type);
	// compute data positions
	m_context << sof::Instruction::SWAP1;
	CompilerUtils(m_context).computeHashStatic();
	// stack: len data_pos
	m_context << sof::Instruction::SWAP1 << sof::Instruction::DUP2 << sof::Instruction::ADD
		<< sof::Instruction::SWAP1;
	// stack: data_pos_end data_pos
	if (_type.isByteArray() || _type.baseType()->storageBytes() < 32)
		clearStorageLoop(IntegerType(256));
	else
		clearStorageLoop(*_type.baseType());
	// cleanup
	m_context << endTag;
	m_context << sof::Instruction::POP;
}

void ArrayUtils::resizeDynamicArray(ArrayType const& _type) const
{
	polAssert(_type.location() == DataLocation::Storage, "");
	polAssert(_type.isDynamicallySized(), "");
	if (!_type.isByteArray() && _type.baseType()->storageBytes() < 32)
		polAssert(_type.baseType()->isValueType(), "Invalid storage size for non-value type.");

	unsigned stackHeightStart = m_context.stackHeight();
	sof::AssemblyItem resizeEnd = m_context.newTag();

	// stack: ref new_length
	// fetch old length
	retrieveLength(_type, 1);
	// stack: ref new_length old_length
	polAssert(m_context.stackHeight() - stackHeightStart == 3 - 2, "2");

	// Special case for short byte arrays, they are stored together with their length
	if (_type.isByteArray())
	{
		sof::AssemblyItem regularPath = m_context.newTag();
		// We start by a large case-distinction about the old and new length of the byte array.

		m_context << sof::Instruction::DUP3 << sof::Instruction::SLOAD;
		// stack: ref new_length current_length ref_value

		polAssert(m_context.stackHeight() - stackHeightStart == 4 - 2, "3");
		m_context << sof::Instruction::DUP2 << u256(31) << sof::Instruction::LT;
		sof::AssemblyItem currentIsLong = m_context.appendConditionalJump();
		m_context << sof::Instruction::DUP3 << u256(31) << sof::Instruction::LT;
		sof::AssemblyItem newIsLong = m_context.appendConditionalJump();

		// Here: short -> short

		// Compute 1 << (256 - 8 * new_size)
		sof::AssemblyItem shortToShort = m_context.newTag();
		m_context << shortToShort;
		m_context << sof::Instruction::DUP3 << u256(8) << sof::Instruction::MUL;
		m_context << u256(0x100) << sof::Instruction::SUB;
		m_context << u256(2) << sof::Instruction::EXP;
		// Divide and multiply by that value, clearing bits.
		m_context << sof::Instruction::DUP1 << sof::Instruction::SWAP2;
		m_context << sof::Instruction::DIV << sof::Instruction::MUL;
		// Insert 2*length.
		m_context << sof::Instruction::DUP3 << sof::Instruction::DUP1 << sof::Instruction::ADD;
		m_context << sof::Instruction::OR;
		// Store.
		m_context << sof::Instruction::DUP4 << sof::Instruction::SSTORE;
		polAssert(m_context.stackHeight() - stackHeightStart == 3 - 2, "3");
		m_context.appendJumpTo(resizeEnd);

		m_context.adjustStackOffset(1); // we have to do that because of the jumps
		// Here: short -> long

		m_context << newIsLong;
		// stack: ref new_length current_length ref_value
		polAssert(m_context.stackHeight() - stackHeightStart == 4 - 2, "3");
		// Zero out lower-order byte.
		m_context << u256(0xff) << sof::Instruction::NOT << sof::Instruction::AND;
		// Store at data location.
		m_context << sof::Instruction::DUP4;
		CompilerUtils(m_context).computeHashStatic();
		m_context << sof::Instruction::SSTORE;
		// stack: ref new_length current_length
		// Store new length: Compule 2*length + 1 and store it.
		m_context << sof::Instruction::DUP2 << sof::Instruction::DUP1 << sof::Instruction::ADD;
		m_context << u256(1) << sof::Instruction::ADD;
		// stack: ref new_length current_length 2*new_length+1
		m_context << sof::Instruction::DUP4 << sof::Instruction::SSTORE;
		polAssert(m_context.stackHeight() - stackHeightStart == 3 - 2, "3");
		m_context.appendJumpTo(resizeEnd);

		m_context.adjustStackOffset(1); // we have to do that because of the jumps

		m_context << currentIsLong;
		m_context << sof::Instruction::DUP3 << u256(31) << sof::Instruction::LT;
		m_context.appendConditionalJumpTo(regularPath);

		// Here: long -> short
		// Read the first word of the data and store it on the stack. Clear the data location and
		// then jump to the short -> short case.

		// stack: ref new_length current_length ref_value
		polAssert(m_context.stackHeight() - stackHeightStart == 4 - 2, "3");
		m_context << sof::Instruction::POP << sof::Instruction::DUP3;
		CompilerUtils(m_context).computeHashStatic();
		m_context << sof::Instruction::DUP1 << sof::Instruction::SLOAD << sof::Instruction::SWAP1;
		// stack: ref new_length current_length first_word data_location
		m_context << sof::Instruction::DUP3;
		convertLengthToSize(_type);
		m_context << sof::Instruction::DUP2 << sof::Instruction::ADD << sof::Instruction::SWAP1;
		// stack: ref new_length current_length first_word data_location_end data_location
		clearStorageLoop(IntegerType(256));
		m_context << sof::Instruction::POP;
		// stack: ref new_length current_length first_word
		polAssert(m_context.stackHeight() - stackHeightStart == 4 - 2, "3");
		m_context.appendJumpTo(shortToShort);

		m_context << regularPath;
		// stack: ref new_length current_length ref_value
		m_context << sof::Instruction::POP;
	}

	// Change of length for a regular array (i.e. length at location, data at sha3(location)).
	// stack: ref new_length old_length
	// store new length
	m_context << sof::Instruction::DUP2;
	if (_type.isByteArray())
		// For a "long" byte array, store length as 2*length+1
		m_context << sof::Instruction::DUP1 << sof::Instruction::ADD << u256(1) << sof::Instruction::ADD;
	m_context<< sof::Instruction::DUP4 << sof::Instruction::SSTORE;
	// skip if size is not reduced
	m_context << sof::Instruction::DUP2 << sof::Instruction::DUP2
		<< sof::Instruction::ISZERO << sof::Instruction::GT;
	m_context.appendConditionalJumpTo(resizeEnd);

	// size reduced, clear the end of the array
	// stack: ref new_length old_length
	convertLengthToSize(_type);
	m_context << sof::Instruction::DUP2;
	convertLengthToSize(_type);
	// stack: ref new_length old_size new_size
	// compute data positions
	m_context << sof::Instruction::DUP4;
	CompilerUtils(m_context).computeHashStatic();
	// stack: ref new_length old_size new_size data_pos
	m_context << sof::Instruction::SWAP2 << sof::Instruction::DUP3 << sof::Instruction::ADD;
	// stack: ref new_length data_pos new_size delete_end
	m_context << sof::Instruction::SWAP2 << sof::Instruction::ADD;
	// stack: ref new_length delete_end delete_start
	if (_type.isByteArray() || _type.baseType()->storageBytes() < 32)
		clearStorageLoop(IntegerType(256));
	else
		clearStorageLoop(*_type.baseType());

	m_context << resizeEnd;
	// cleanup
	m_context << sof::Instruction::POP << sof::Instruction::POP << sof::Instruction::POP;
	polAssert(m_context.stackHeight() == stackHeightStart - 2, "");
}

void ArrayUtils::clearStorageLoop(Type const& _type) const
{
	unsigned stackHeightStart = m_context.stackHeight();
	if (_type.category() == Type::Category::Mapping)
	{
		m_context << sof::Instruction::POP;
		return;
	}
	// stack: end_pos pos

	// jump to and return from the loop to allow for duplicate code removal
	sof::AssemblyItem returnTag = m_context.pushNewTag();
	m_context << sof::Instruction::SWAP2 << sof::Instruction::SWAP1;

	// stack: <return tag> end_pos pos
	sof::AssemblyItem loopStart = m_context.appendJumpToNew();
	m_context << loopStart;
	// check for loop condition
	m_context << sof::Instruction::DUP1 << sof::Instruction::DUP3
			   << sof::Instruction::GT << sof::Instruction::ISZERO;
	sof::AssemblyItem zeroLoopEnd = m_context.newTag();
	m_context.appendConditionalJumpTo(zeroLoopEnd);
	// delete
	m_context << u256(0);
	StorageItem(m_context, _type).setToZero(SourceLocation(), false);
	m_context << sof::Instruction::POP;
	// increment
	m_context << u256(1) << sof::Instruction::ADD;
	m_context.appendJumpTo(loopStart);
	// cleanup
	m_context << zeroLoopEnd;
	m_context << sof::Instruction::POP << sof::Instruction::SWAP1;
	// "return"
	m_context << sof::Instruction::JUMP;

	m_context << returnTag;
	polAssert(m_context.stackHeight() == stackHeightStart - 1, "");
}

void ArrayUtils::convertLengthToSize(ArrayType const& _arrayType, bool _pad) const
{
	if (_arrayType.location() == DataLocation::Storage)
	{
		if (_arrayType.baseType()->storageSize() <= 1)
		{
			unsigned baseBytes = _arrayType.baseType()->storageBytes();
			if (baseBytes == 0)
				m_context << sof::Instruction::POP << u256(1);
			else if (baseBytes <= 16)
			{
				unsigned itemsPerSlot = 32 / baseBytes;
				m_context
					<< u256(itemsPerSlot - 1) << sof::Instruction::ADD
					<< u256(itemsPerSlot) << sof::Instruction::SWAP1 << sof::Instruction::DIV;
			}
		}
		else
			m_context << _arrayType.baseType()->storageSize() << sof::Instruction::MUL;
	}
	else
	{
		if (!_arrayType.isByteArray())
		{
			if (_arrayType.location() == DataLocation::Memory)
				m_context << _arrayType.baseType()->memoryHeadSize();
			else
				m_context << _arrayType.baseType()->calldataEncodedSize();
			m_context << sof::Instruction::MUL;
		}
		else if (_pad)
			m_context << u256(31) << sof::Instruction::ADD
				<< u256(32) << sof::Instruction::DUP1
				<< sof::Instruction::SWAP2 << sof::Instruction::DIV << sof::Instruction::MUL;
	}
}

void ArrayUtils::retrieveLength(ArrayType const& _arrayType, unsigned _stackDepth) const
{
	if (!_arrayType.isDynamicallySized())
		m_context << _arrayType.length();
	else
	{
		m_context << sof::dupInstruction(1 + _stackDepth);
		switch (_arrayType.location())
		{
		case DataLocation::CallData:
			// length is stored on the stack
			break;
		case DataLocation::Memory:
			m_context << sof::Instruction::MLOAD;
			break;
		case DataLocation::Storage:
			m_context << sof::Instruction::SLOAD;
			if (_arrayType.isByteArray())
			{
				// Retrieve length both for in-place strings and off-place strings:
				// Computes (x & (0x100 * (ISZERO (x & 1)) - 1)) / 2
				// i.e. for short strings (x & 1 == 0) it does (x & 0xff) / 2 and for long strings it
				// computes (x & (-1)) / 2, which is equivalent to just x / 2.
				m_context << u256(1) << sof::Instruction::DUP2 << u256(1) << sof::Instruction::AND;
				m_context << sof::Instruction::ISZERO << u256(0x100) << sof::Instruction::MUL;
				m_context << sof::Instruction::SUB << sof::Instruction::AND;
				m_context << u256(2) << sof::Instruction::SWAP1 << sof::Instruction::DIV;
			}
			break;
		}
	}
}

void ArrayUtils::accessIndex(ArrayType const& _arrayType, bool _doBoundsCheck) const
{
	/// Stack: reference [length] index
	DataLocation location = _arrayType.location();

	if (_doBoundsCheck)
	{
		// retrieve length
		ArrayUtils::retrieveLength(_arrayType, 1);
		// Stack: ref [length] index length
		// check out-of-bounds access
		m_context << sof::Instruction::DUP2 << sof::Instruction::LT << sof::Instruction::ISZERO;
		// out-of-bounds access throws exception
		m_context.appendConditionalJumpTo(m_context.errorTag());
	}
	if (location == DataLocation::CallData && _arrayType.isDynamicallySized())
		// remove length if present
		m_context << sof::Instruction::SWAP1 << sof::Instruction::POP;

	// stack: <base_ref> <index>
	m_context << sof::Instruction::SWAP1;
	// stack: <index> <base_ref>
	switch (location)
	{
	case DataLocation::Memory:
		if (_arrayType.isDynamicallySized())
			m_context << u256(32) << sof::Instruction::ADD;
		// fall-through
	case DataLocation::CallData:
		if (!_arrayType.isByteArray())
		{
			m_context << sof::Instruction::SWAP1;
			if (location == DataLocation::CallData)
				m_context << _arrayType.baseType()->calldataEncodedSize();
			else
				m_context << u256(_arrayType.memoryHeadSize());
			m_context << sof::Instruction::MUL;
		}
		m_context << sof::Instruction::ADD;
		break;
	case DataLocation::Storage:
	{
		sof::AssemblyItem endTag = m_context.newTag();
		if (_arrayType.isByteArray())
		{
			// Special case of short byte arrays.
			m_context << sof::Instruction::SWAP1;
			m_context << sof::Instruction::DUP2 << sof::Instruction::SLOAD;
			m_context << u256(1) << sof::Instruction::AND << sof::Instruction::ISZERO;
			// No action needed for short byte arrays.
			m_context.appendConditionalJumpTo(endTag);
			m_context << sof::Instruction::SWAP1;
		}
		if (_arrayType.isDynamicallySized())
			CompilerUtils(m_context).computeHashStatic();
		m_context << sof::Instruction::SWAP1;
		if (_arrayType.baseType()->storageBytes() <= 16)
		{
			// stack: <data_ref> <index>
			// goal:
			// <ref> <byte_number> = <base_ref + index / itemsPerSlot> <(index % itemsPerSlot) * byteSize>
			unsigned byteSize = _arrayType.baseType()->storageBytes();
			polAssert(byteSize != 0, "");
			unsigned itemsPerSlot = 32 / byteSize;
			m_context << u256(itemsPerSlot) << sof::Instruction::SWAP2;
			// stack: itemsPerSlot index data_ref
			m_context
				<< sof::Instruction::DUP3 << sof::Instruction::DUP3
				<< sof::Instruction::DIV << sof::Instruction::ADD
			// stack: itemsPerSlot index (data_ref + index / itemsPerSlot)
				<< sof::Instruction::SWAP2 << sof::Instruction::SWAP1
				<< sof::Instruction::MOD;
			if (byteSize != 1)
				m_context << u256(byteSize) << sof::Instruction::MUL;
		}
		else
		{
			if (_arrayType.baseType()->storageSize() != 1)
				m_context << _arrayType.baseType()->storageSize() << sof::Instruction::MUL;
			m_context << sof::Instruction::ADD << u256(0);
		}
		m_context << endTag;
		break;
	}
	default:
		polAssert(false, "");
	}
}

void ArrayUtils::incrementByteOffset(unsigned _byteSize, unsigned _byteOffsetPosition, unsigned _storageOffsetPosition) const
{
	polAssert(_byteSize < 32, "");
	polAssert(_byteSize != 0, "");
	// We do the following, but avoiding jumps:
	// byteOffset += byteSize
	// if (byteOffset + byteSize > 32)
	// {
	//     storageOffset++;
	//     byteOffset = 0;
	// }
	if (_byteOffsetPosition > 1)
		m_context << sof::swapInstruction(_byteOffsetPosition - 1);
	m_context << u256(_byteSize) << sof::Instruction::ADD;
	if (_byteOffsetPosition > 1)
		m_context << sof::swapInstruction(_byteOffsetPosition - 1);
	// compute, X := (byteOffset + byteSize - 1) / 32, should be 1 iff byteOffset + bytesize > 32
	m_context
		<< u256(32) << sof::dupInstruction(1 + _byteOffsetPosition) << u256(_byteSize - 1)
		<< sof::Instruction::ADD << sof::Instruction::DIV;
	// increment storage offset if X == 1 (just add X to it)
	// stack: X
	m_context
		<< sof::swapInstruction(_storageOffsetPosition) << sof::dupInstruction(_storageOffsetPosition + 1)
		<< sof::Instruction::ADD << sof::swapInstruction(_storageOffsetPosition);
	// stack: X
	// set source_byte_offset to zero if X == 1 (using source_byte_offset *= 1 - X)
	m_context << u256(1) << sof::Instruction::SUB;
	// stack: 1 - X
	if (_byteOffsetPosition == 1)
		m_context << sof::Instruction::MUL;
	else
		m_context
			<< sof::dupInstruction(_byteOffsetPosition + 1) << sof::Instruction::MUL
			<< sof::swapInstruction(_byteOffsetPosition) << sof::Instruction::POP;
}