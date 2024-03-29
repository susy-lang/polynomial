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
 * @file SemanticInformation.h
 * @author Christian <c@sofdev.com>
 * @date 2015
 * Helper to provide semantic information about assembly items.
 */

#pragma once

#include <libsvmasm/Instruction.h>

namespace dev
{
namespace sof
{

class AssemblyItem;

/**
 * Helper functions to provide context-independent information about assembly items.
 */
struct SemanticInformation
{
	/// @returns true if the given items starts a new block for common subexpression analysis.
	/// @param _msizeImportant if false, consider an operation non-breaking if its only side-effect is that it modifies msize.
	static bool breaksCSEAnalysisBlock(AssemblyItem const& _item, bool _msizeImportant);
	/// @returns true if the item is a two-argument operation whose value does not depend on the
	/// order of its arguments.
	static bool isCommutativeOperation(AssemblyItem const& _item);
	static bool isDupInstruction(AssemblyItem const& _item);
	static bool isSwapInstruction(AssemblyItem const& _item);
	static bool isJumpInstruction(AssemblyItem const& _item);
	static bool altersControlFlow(AssemblyItem const& _item);
	static bool terminatesControlFlow(AssemblyItem const& _item);
	static bool terminatesControlFlow(Instruction _instruction);
	/// @returns false if the value put on the stack by _item depends on anything else than
	/// the information in the current block header, memory, storage or stack.
	static bool isDeterministic(AssemblyItem const& _item);
	/// @returns true if the instruction can be moved or copied (together with its arguments)
	/// without altering the semantics. This means it cannot depend on storage or memory,
	/// cannot have any side-effects, but it can depend on a call-constant state of the blockchain.
	static bool movable(Instruction _instruction);
	/// @returns true if the instruction can be removed without changing the semantics.
	/// This does not mean that it has to be deterministic or retrieve information from
	/// somewhere else than purely the values of its arguments.
	static bool sideEffectFree(Instruction _instruction);
	/// @returns true if the instruction can be removed without changing the semantics.
	/// This does not mean that it has to be deterministic or retrieve information from
	/// somewhere else than purely the values of its arguments.
	/// If true, the instruction is still allowed to influence the value returned by the
	/// msize instruction.
	static bool sideEffectFreeIfNoMSize(Instruction _instruction);
	/// @returns true if the given instruction modifies memory.
	static bool invalidatesMemory(Instruction _instruction);
	/// @returns true if the given instruction modifies storage (even indirectly).
	static bool invalidatesStorage(Instruction _instruction);
	static bool invalidInPureFunctions(Instruction _instruction);
	static bool invalidInViewFunctions(Instruction _instruction);
};

}
}
