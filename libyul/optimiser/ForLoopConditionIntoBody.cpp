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

#include <libyul/optimiser/ForLoopConditionIntoBody.h>
#include <libyul/AsmData.h>
#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;

void ForLoopConditionIntoBody::operator()(ForLoop& _forLoop)
{
	if (_forLoop.condition->type() != typeid(Literal))
	{
		langutil::SourceLocation loc = locationOf(*_forLoop.condition);
		_forLoop.body.statements.insert(
			_forLoop.body.statements.begin(),
			If {
				loc,
				make_unique<Expression>(
					FunctionalInstruction {
						loc,
						sof::Instruction::ISZERO,
						make_vector<Expression>(std::move(*_forLoop.condition))
					}
				),
				Block {loc, make_vector<Statement>(Break{{}})}
			}
		);
		_forLoop.condition = make_unique<Expression>(
			Literal {
				loc,
				LiteralKind::Number,
				"1"_yulstring,
				{}
			}
		);
	}
	ASTModifier::operator()(_forLoop);
}

