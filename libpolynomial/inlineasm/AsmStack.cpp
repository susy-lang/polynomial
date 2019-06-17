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

#include <libpolynomial/inlineasm/AsmParser.h>
#include <libpolynomial/inlineasm/AsmCodeGen.h>
#include <libpolynomial/inlineasm/AsmPrinter.h>
#include <libpolynomial/inlineasm/AsmAnalysis.h>
#include <libpolynomial/inlineasm/AsmAnalysisInfo.h>

#include <libpolynomial/parsing/Scanner.h>

#include <libsvmasm/Assembly.h>
#include <libsvmasm/SourceLocation.h>

#include <memory>

using namespace std;
using namespace dev;
using namespace dev::polynomial;
using namespace dev::polynomial::assembly;

bool InlineAssemblyStack::parse(
	shared_ptr<Scanner> const& _scanner,
	ExternalIdentifierAccess::Resolver const& _resolver
)
{
	m_parserResult = make_shared<Block>();
	Parser parser(m_errors);
	auto result = parser.parse(_scanner);
	if (!result)
		return false;

	*m_parserResult = std::move(*result);
	AsmAnalysisInfo analysisInfo;
	return (AsmAnalyzer(analysisInfo, m_errors, _resolver)).analyze(*m_parserResult);
}

string InlineAssemblyStack::toString()
{
	return AsmPrinter()(*m_parserResult);
}

sof::Assembly InlineAssemblyStack::assemble()
{
	AsmAnalysisInfo analysisInfo;
	AsmAnalyzer analyzer(analysisInfo, m_errors);
	polAssert(analyzer.analyze(*m_parserResult), "");
	CodeGenerator codeGen(m_errors);
	return codeGen.assemble(*m_parserResult, analysisInfo);
}

bool InlineAssemblyStack::parseAndAssemble(
	string const& _input,
	sof::Assembly& _assembly,
	ExternalIdentifierAccess const& _identifierAccess
)
{
	ErrorList errors;
	auto scanner = make_shared<Scanner>(CharStream(_input), "--CODEGEN--");
	auto parserResult = Parser(errors).parse(scanner);
	if (!errors.empty())
		return false;
	polAssert(parserResult, "");

	AsmAnalysisInfo analysisInfo;
	AsmAnalyzer analyzer(analysisInfo, errors, _identifierAccess.resolve);
	polAssert(analyzer.analyze(*parserResult), "");
	CodeGenerator(errors).assemble(*parserResult, analysisInfo, _assembly, _identifierAccess);

	// At this point, the assembly might be messed up, but we should throw an
	// internal compiler error anyway.
	return errors.empty();
}

