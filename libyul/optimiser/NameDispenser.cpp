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

#include <libyul/optimiser/NameDispenser.h>

#include <libyul/optimiser/NameCollector.h>
#include <libyul/AsmData.h>
#include <libyul/Dialect.h>
#include <libyul/backends/svm/SVMDialect.h>
#include <libyul/AsmParser.h>

#include <libsvmasm/Instruction.h>

using namespace std;
using namespace dev;
using namespace yul;

NameDispenser::NameDispenser(Dialect const& _dialect, Block const& _ast):
	NameDispenser(_dialect, NameCollector(_ast).names())
{
}

NameDispenser::NameDispenser(Dialect const& _dialect, set<YulString> _usedNames):
	m_dialect(_dialect),
	m_usedNames(std::move(_usedNames))
{
}

YulString NameDispenser::newName(YulString _nameHint)
{
	YulString name = _nameHint;
	while (illegalName(name))
	{
		m_counter++;
		name = YulString(_nameHint.str() + "_" + to_string(m_counter));
	}
	m_usedNames.emplace(name);
	return name;
}

bool NameDispenser::illegalName(YulString _name)
{
	if (_name.empty() || m_usedNames.count(_name) || m_dialect.builtin(_name))
		return true;
	if (dynamic_cast<SVMDialect const*>(&m_dialect))
		return Parser::instructions().count(_name.str());
	return false;
}
