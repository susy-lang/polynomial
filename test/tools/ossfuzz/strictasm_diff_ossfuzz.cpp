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

#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/Dialect.h>
#include <libyul/backends/svm/SVMDialect.h>
#include <libyul/AssemblyStack.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/SVMVersion.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libdevcore/CommonIO.h>
#include <libdevcore/CommonData.h>

#include <test/tools/ossfuzz/yulFuzzerCommon.h>

#include <string>
#include <memory>
#include <iostream>

using namespace yul;
using namespace std;

using namespace langutil;
using namespace dev;
using namespace yul::test::yul_fuzzer;

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* _data, size_t _size)
{
	if (_size > 600)
		return 0;

	string input(reinterpret_cast<char const*>(_data), _size);

	AssemblyStack stack(SVMVersion::petersburg(), AssemblyStack::Language::StrictAssembly);
	try
	{
		if (!stack.parseAndAnalyze("source", input) || !stack.parserResult()->code ||
			!stack.parserResult()->analysisInfo)
			return 0;
	}
	catch (Exception const&)
	{
		return 0;
	}

	ostringstream os1;
	ostringstream os2;
	yulFuzzerUtil::interpret(os1, stack.parserResult()->code);
	stack.optimize();
	yulFuzzerUtil::interpret(os2, stack.parserResult()->code);

	bool isTraceEq = (os1.str() == os2.str());
	yulAssert(isTraceEq, "Interpreted traces for optimized and unoptimized code differ.");
	return 0;
}