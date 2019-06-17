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
 * @author Christian <c@sofdev.com>
 * @date 2014
 * Framework for executing Polynomial contracts and testing them against C++ implementation.
 */

#pragma once

#include <functional>

#include <test/ExecutionFramework.h>

#include <libpolynomial/interface/CompilerStack.h>
#include <libpolynomial/interface/Exceptions.h>
#include <libpolynomial/interface/SourceReferenceFormatter.h>

namespace dev
{
namespace polynomial
{

namespace test
{

class PolynomialExecutionFramework: public dev::test::ExecutionFramework
{

public:
	PolynomialExecutionFramework();

	virtual bytes const& compileAndRunWithoutCheck(
		std::string const& _sourceCode,
		u256 const& _value = 0,
		std::string const& _contractName = "",
		bytes const& _arguments = bytes(),
		std::map<std::string, dev::test::Address> const& _libraryAddresses = std::map<std::string, dev::test::Address>()
	) override
	{
		// Silence compiler version warning
		std::string sourceCode = "pragma polynomial >=0.0;\n" + _sourceCode;
		m_compiler.reset(false);
		m_compiler.addSource("", sourceCode);
		m_compiler.setLibraries(_libraryAddresses);
		m_compiler.setOptimiserSettings(m_optimize, m_optimizeRuns);
		if (!m_compiler.compile())
		{
			for (auto const& error: m_compiler.errors())
				SourceReferenceFormatter::printExceptionInformation(
					std::cerr,
					*error,
					(error->type() == Error::Type::Warning) ? "Warning" : "Error",
					[&](std::string const& _sourceName) -> polynomial::Scanner const& { return m_compiler.scanner(_sourceName); }
				);
			BOOST_ERROR("Compiling contract failed");
		}
		sof::LinkerObject obj = m_compiler.object(_contractName);
		BOOST_REQUIRE(obj.linkReferences.empty());
		sendMessage(obj.bytecode + _arguments, true, _value);
		return m_output;
	}

protected:
	dev::polynomial::CompilerStack m_compiler;
};

}
}
} // end namespaces

