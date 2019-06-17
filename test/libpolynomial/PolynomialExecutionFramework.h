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

#include <libyul/AssemblyStack.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/SourceReferenceFormatter.h>

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
	PolynomialExecutionFramework(std::string const& _ipcPath, langutil::SVMVersion _svmVersion);

	virtual bytes const& compileAndRunWithoutCheck(
		std::string const& _sourceCode,
		u256 const& _value = 0,
		std::string const& _contractName = "",
		bytes const& _arguments = bytes(),
		std::map<std::string, dev::test::Address> const& _libraryAddresses = std::map<std::string, dev::test::Address>()
	) override
	{
		bytes bytecode = compileContract(_sourceCode, _contractName, _libraryAddresses);
		sendMessage(bytecode + _arguments, true, _value);
		return m_output;
	}

	bytes compileContract(
		std::string const& _sourceCode,
		std::string const& _contractName = "",
		std::map<std::string, dev::test::Address> const& _libraryAddresses = std::map<std::string, dev::test::Address>()
	);

protected:
	dev::polynomial::CompilerStack m_compiler;
	bool m_compileViaYul = false;
};

}
}
} // end namespaces

