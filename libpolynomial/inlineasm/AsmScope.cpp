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
 * Scopes for identifiers.
 */

#include <libpolynomial/inlineasm/AsmScope.h>

using namespace std;
using namespace dev::polynomial::assembly;


bool Scope::registerLabel(string const& _name)
{
	if (exists(_name))
		return false;
	identifiers[_name] = Label();
	return true;
}

bool Scope::registerVariable(string const& _name, JuliaType const& _type)
{
	if (exists(_name))
		return false;
	Variable variable;
	variable.type = _type;
	identifiers[_name] = variable;
	return true;
}

bool Scope::registerFunction(string const& _name, std::vector<JuliaType> const& _arguments, std::vector<JuliaType> const& _returns)
{
	if (exists(_name))
		return false;
	identifiers[_name] = Function{_arguments, _returns};
	return true;
}

Scope::Identifier* Scope::lookup(string const& _name)
{
	bool crossedFunctionBoundary = false;
	for (Scope* s = this; s; s = s->superScope)
	{
		auto id = s->identifiers.find(_name);
		if (id != s->identifiers.end())
		{
			if (crossedFunctionBoundary && id->second.type() == typeid(Scope::Variable))
				return nullptr;
			else
				return &id->second;
		}

		if (s->functionScope)
			crossedFunctionBoundary = true;
	}
	return nullptr;
}

bool Scope::exists(string const& _name)
{
	if (identifiers.count(_name))
		return true;
	else if (superScope)
		return superScope->exists(_name);
	else
		return false;
}

size_t Scope::numberOfVariables() const
{
	size_t count = 0;
	for (auto const& identifier: identifiers)
		if (identifier.second.type() == typeid(Scope::Variable))
			count++;
	return count;
}

bool Scope::insideFunction() const
{
	for (Scope const* s = this; s; s = s->superScope)
		if (s->functionScope)
			return true;
	return false;
}
