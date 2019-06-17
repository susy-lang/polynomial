/*(
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
 * Component that checks whether all variables are reachable on the stack.
 */

#include <libyul/CompilabilityChecker.h>

#include <libyul/AsmAnalysis.h>
#include <libyul/AsmAnalysisInfo.h>

#include <libyul/backends/svm/SVMCodeTransform.h>
#include <libyul/backends/svm/NoOutputAssembly.h>

#include <liblangutil/SVMVersion.h>

using namespace std;
using namespace yul;
using namespace dev;
using namespace dev::polynomial;

map<YulString, int> CompilabilityChecker::run(
	shared_ptr<Dialect> _dialect,
	Block const& _ast,
	bool _optimizeStackAllocation
)
{
	if (_dialect->flavour == AsmFlavour::Yul)
		return {};

	polAssert(_dialect->flavour == AsmFlavour::Strict, "");

	polAssert(dynamic_cast<SVMDialect const*>(_dialect.get()), "");
	shared_ptr<NoOutputSVMDialect> noOutputDialect = make_shared<NoOutputSVMDialect>(dynamic_pointer_cast<SVMDialect>(_dialect));

	yul::AsmAnalysisInfo analysisInfo =
		yul::AsmAnalyzer::analyzeStrictAssertCorrect(noOutputDialect, _ast);

	NoOutputAssembly assembly;
	CodeTransform transform(assembly, analysisInfo, _ast, *noOutputDialect, _optimizeStackAllocation);
	try
	{
		transform(_ast);
	}
	catch (StackTooDeepError const&)
	{
		polAssert(!transform.stackErrors().empty(), "Got stack too deep exception that was not stored.");
	}

	std::map<YulString, int> functions;
	for (StackTooDeepError const& error: transform.stackErrors())
		functions[error.functionName] = max(error.depth, functions[error.functionName]);

	return functions;
}
