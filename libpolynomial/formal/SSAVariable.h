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

#pragma once

#include <memory>

namespace dev
{
namespace polynomial
{
namespace smt
{

/**
 * This class represents the SSA representation of a program variable.
 */
class SSAVariable
{
public:
	SSAVariable();
	void resetIndex();

	/// This function returns the current index of this SSA variable.
	unsigned index() const { return m_currentIndex; }
	unsigned& index() { return m_currentIndex; }

	unsigned operator++()
	{
		return m_currentIndex = (*m_nextFreeIndex)++;
	}

private:
	unsigned m_currentIndex;
	std::unique_ptr<unsigned> m_nextFreeIndex;
};

}
}
}
