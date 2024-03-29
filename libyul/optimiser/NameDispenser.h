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
 * Optimiser component that can create new unique names.
 */
#pragma once

#include <libyul/AsmDataForward.h>

#include <libyul/YulString.h>

#include <set>

namespace yul
{
struct Dialect;

/**
 * Optimizer component that can be used to generate new names that
 * do not conflict with existing names.
 *
 * Tries to keep names short and appends decimals to disambiguate.
 */
class NameDispenser
{
public:
	/// Initialize the name dispenser with all the names used in the given AST.
	explicit NameDispenser(Dialect const& _dialect, Block const& _ast);
	/// Initialize the name dispenser with the given used names.
	explicit NameDispenser(Dialect const& _dialect, std::set<YulString> _usedNames);

	/// @returns a currently unused name that should be similar to _nameHint.
	YulString newName(YulString _nameHint);

private:
	bool illegalName(YulString _name);

	Dialect const& m_dialect;
	std::set<YulString> m_usedNames;
	size_t m_counter = 0;
};

}
