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
 * Yul interpreter module that evaluates SVM instructions.
 */

#pragma once

#include <libyul/AsmDataForward.h>

#include <libdevcore/CommonData.h>

#include <vector>

namespace dev
{
namespace sof
{
enum class Instruction: uint8_t;
}
}

namespace yul
{
class YulString;
struct BuiltinFunctionForSVM;

namespace test
{

struct InterpreterState;

/**
 * Interprets SVM instructions based on the current state and logs instructions with
 * side-effects.
 *
 * Since this is mainly meant to be used for differential fuzz testing, it is focused
 * on a single contract only, does not do any gas counting and differs from the correct
 * implementation in many ways:
 *
 * - If memory access to a "large" memory position is performed, a deterministic
 *   value is returned. Data that is stored in a "large" memory position is not
 *   retained.
 * - The blockhash instruction returns a fixed value if the argument is in range.
 * - Extcodesize returns a deterministic value depending on the address.
 * - Extcodecopy copies a deterministic value depending on the address.
 * - And many other things
 *
 * The main focus is that the generated execution trace is the same for equivalent executions
 * and likely to be different for non-equivalent executions.
 */
class SVMInstructionInterpreter
{
public:
	explicit SVMInstructionInterpreter(InterpreterState& _state):
		m_state(_state)
	{}
	/// Evaluate instruction
	dev::u256 eval(dev::sof::Instruction _instruction, std::vector<dev::u256> const& _arguments);
	/// Evaluate builtin function
	dev::u256 evalBuiltin(BuiltinFunctionForSVM const& _fun, std::vector<dev::u256> const& _arguments);

private:
	/// Resizes the memory to accommodate the memory access.
	/// @returns false if memory would have to be expanded beyond m_state.maxMemSize.
	bool accessMemory(dev::u256 const& _offset, dev::u256 const& _size = 32);

	void logTrace(dev::sof::Instruction _instruction, std::vector<dev::u256> const& _arguments = {}, dev::bytes const& _data = {});
	/// Appends a log to the trace representing an instruction or similar operation by string,
	/// with arguments and auxiliary data (if nonempty).
	void logTrace(std::string const& _pseudoInstruction, std::vector<dev::u256> const& _arguments = {}, dev::bytes const& _data = {});

	InterpreterState& m_state;
};

}
}
