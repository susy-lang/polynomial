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
 * Common code generator for translating Yul / inline assembly to SVM and SVM1.5.
 */

#include <libyul/backends/svm/SVMAssembly.h>

#include <libyul/ASTDataForward.h>

#include <libpolynomial/inlineasm/AsmScope.h>

#include <boost/variant.hpp>
#include <boost/optional.hpp>

namespace dev
{
namespace polynomial
{
class ErrorReporter;
namespace assembly
{
struct AsmAnalysisInfo;
}
}
namespace yul
{
class SVMAssembly;

class CodeTransform: public boost::static_visitor<>
{
public:
	/// Create the code transformer.
	/// @param _identifierAccess used to resolve identifiers external to the inline assembly
	CodeTransform(
		yul::AbstractAssembly& _assembly,
		polynomial::assembly::AsmAnalysisInfo& _analysisInfo,
		bool _yul = false,
		bool _svm15 = false,
		ExternalIdentifierAccess const& _identifierAccess = ExternalIdentifierAccess(),
		bool _useNamedLabelsForFunctions = false
	): CodeTransform(
		_assembly,
		_analysisInfo,
		_yul,
		_svm15,
		_identifierAccess,
		_useNamedLabelsForFunctions,
		_assembly.stackHeight(),
		std::make_shared<Context>()
	)
	{
	}

protected:
	struct Context
	{
		using Scope = polynomial::assembly::Scope;
		std::map<Scope::Label const*, AbstractAssembly::LabelID> labelIDs;
		std::map<Scope::Function const*, AbstractAssembly::LabelID> functionEntryIDs;
		std::map<Scope::Variable const*, int> variableStackHeights;
	};

	CodeTransform(
		yul::AbstractAssembly& _assembly,
		polynomial::assembly::AsmAnalysisInfo& _analysisInfo,
		bool _yul,
		bool _svm15,
		ExternalIdentifierAccess const& _identifierAccess,
		bool _useNamedLabelsForFunctions,
		int _stackAdjustment,
		std::shared_ptr<Context> _context
	):
		m_assembly(_assembly),
		m_info(_analysisInfo),
		m_yul(_yul),
		m_svm15(_svm15),
		m_useNamedLabelsForFunctions(_useNamedLabelsForFunctions),
		m_identifierAccess(_identifierAccess),
		m_stackAdjustment(_stackAdjustment),
		m_context(_context)
	{}

public:
	void operator()(Instruction const& _instruction);
	void operator()(Literal const& _literal);
	void operator()(Identifier const& _identifier);
	void operator()(FunctionalInstruction const& _instr);
	void operator()(FunctionCall const&);
	void operator()(ExpressionStatement const& _statement);
	void operator()(Label const& _label);
	void operator()(StackAssignment const& _assignment);
	void operator()(Assignment const& _assignment);
	void operator()(VariableDeclaration const& _varDecl);
	void operator()(If const& _if);
	void operator()(Switch const& _switch);
	void operator()(FunctionDefinition const&);
	void operator()(ForLoop const&);
	void operator()(Block const& _block);

private:
	AbstractAssembly::LabelID labelFromIdentifier(Identifier const& _identifier);
	/// @returns the label ID corresponding to the given label, allocating a new one if
	/// necessary.
	AbstractAssembly::LabelID labelID(polynomial::assembly::Scope::Label const& _label);
	AbstractAssembly::LabelID functionEntryID(YulString _name, polynomial::assembly::Scope::Function const& _function);
	/// Generates code for an expression that is supposed to return a single value.
	void visitExpression(Expression const& _expression);

	void visitStatements(std::vector<Statement> const& _statements);

	/// Pops all variables declared in the block and checks that the stack height is equal
	/// to @a _blackStartStackHeight.
	void finalizeBlock(Block const& _block, int _blockStartStackHeight);

	void generateMultiAssignment(std::vector<Identifier> const& _variableNames);
	void generateAssignment(Identifier const& _variableName);

	/// Determines the stack height difference to the given variables. Throws
	/// if it is not yet in scope or the height difference is too large. Returns
	/// the (positive) stack height difference otherwise.
	int variableHeightDiff(polynomial::assembly::Scope::Variable const& _var, bool _forSwap) const;

	void expectDeposit(int _deposit, int _oldHeight) const;

	void checkStackHeight(void const* _astElement) const;

	yul::AbstractAssembly& m_assembly;
	polynomial::assembly::AsmAnalysisInfo& m_info;
	polynomial::assembly::Scope* m_scope = nullptr;
	bool m_yul = false;
	bool m_svm15 = false;
	bool m_useNamedLabelsForFunctions = false;
	ExternalIdentifierAccess m_identifierAccess;
	/// Adjustment between the stack height as determined during the analysis phase
	/// and the stack height in the assembly. This is caused by an initial stack being present
	/// for inline assembly and different stack heights depending on the SVM backend used
	/// (SVM 1.0 or 1.5).
	int m_stackAdjustment = 0;
	std::shared_ptr<Context> m_context;
};

}
}
