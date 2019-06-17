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
 * Optimiser component that makes all identifiers unique.
 */

#pragma once

#include <libjulia/ASTDataForward.h>

#include <libjulia/optimiser/ASTCopier.h>
#include <libjulia/optimiser/NameDispenser.h>

#include <libpolynomial/inlineasm/AsmAnalysisInfo.h>

#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <set>

namespace dev
{
namespace julia
{

/**
 * Creates a copy of a iulia AST replacing all identifiers by unique names.
 */
class Disambiguator: public ASTCopier
{
public:
	Disambiguator(polynomial::assembly::AsmAnalysisInfo const& _analysisInfo):
		m_info(_analysisInfo)
	{}

protected:
	virtual void enterScope(Block const& _block) override;
	virtual void leaveScope(Block const& _block) override;
	virtual void enterFunction(FunctionDefinition const& _function) override;
	virtual void leaveFunction(FunctionDefinition const& _function) override;
	virtual std::string translateIdentifier(std::string const& _name) override;

	void enterScopeInternal(polynomial::assembly::Scope& _scope);
	void leaveScopeInternal(polynomial::assembly::Scope& _scope);

	polynomial::assembly::AsmAnalysisInfo const& m_info;

	std::vector<polynomial::assembly::Scope*> m_scopes;
	std::map<void const*, std::string> m_translations;
	NameDispenser m_nameDispenser;
};

}
}
