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
 * @date 2016
 * Full-stack Polynomial inline assember.
 */

#include <libpolynomial/inlineasm/AsmStack.h>
#include <memory>
#include <libsvmasm/Assembly.h>
#include <libsvmasm/SourceLocation.h>
#include <libpolynomial/parsing/Scanner.h>
#include <libpolynomial/inlineasm/AsmParser.h>
#include <libpolynomial/inlineasm/AsmCodeGen.h>

using namespace std;
using namespace dev;
using namespace dev::polynomial;
using namespace dev::polynomial::assembly;

bool InlineAssemblyStack::parse(shared_ptr<Scanner> const& _scanner)
{
	m_parserResult = make_shared<Block>();
	Parser parser(m_errors);
	auto result = parser.parse(_scanner);
	if (!result)
		return false;
	*m_parserResult = std::move(*result);
	return true;
}

sof::Assembly InlineAssemblyStack::assemble()
{
	CodeGenerator codeGen(*m_parserResult, m_errors);
	return codeGen.assemble();
}

bool InlineAssemblyStack::parseAndAssemble(
	string const& _input,
	sof::Assembly& _assembly,
	CodeGenerator::IdentifierAccess const& _identifierAccess
)
{
	ErrorList errors;
	auto scanner = make_shared<Scanner>(CharStream(_input), "--CODEGEN--");
	auto parserResult = Parser(errors).parse(scanner);
	if (!errors.empty())
		return false;

	CodeGenerator(*parserResult, errors).assemble(_assembly, _identifierAccess);

	// At this point, the assembly might be messed up, but we should throw an
	// internal compiler error anyway.
	return errors.empty();
}

