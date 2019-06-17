/*
	This file is part of cpp-sophon.

	cpp-sophon is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-sophon is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MSRCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-sophon.  If not, see <http://www.gnu.org/licenses/>.
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
#include <libpolynomial/inlineasm/AsmParser.h>
#include <libpolynomial/inlineasm/AsmCodeGen.h>

using namespace std;
using namespace dev;
using namespace dev::polynomial;
using namespace dev::polynomial::assembly;

bool InlineAssemblyStack::parse(const std::shared_ptr<Scanner>& _scanner)
{
	Parser parser(m_errors);
	m_asmBlock = parser.parse(_scanner);
	return !!m_asmBlock;
}

sof::Assembly InlineAssemblyStack::assemble()
{
	CodeGenerator codeGen(*m_asmBlock, m_errors);
	return codeGen.assemble();
}

