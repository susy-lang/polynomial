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
 * Common code generator for translating Julia / inline assembly to SVM and SVM1.5.
 */

#include <libjulia/backends/svm/SVMAssembly.h>

#include <libpolynomial/inlineasm/AsmScope.h>
#include <libpolynomial/inlineasm/AsmDataForward.h>

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
namespace julia
{
class SVMAssembly;

class CodeTransform: public boost::static_visitor<>
{
public:
	/// Create the code transformer.
	/// @param _identifierAccess used to resolve identifiers external to the inline assembly
	CodeTransform(
		julia::AbstractAssembly& _assembly,
		polynomial::assembly::AsmAnalysisInfo& _analysisInfo,
		bool _julia = false,
		bool _svm15 = false,
		ExternalIdentifierAccess const& _identifierAccess = ExternalIdentifierAccess()
	): CodeTransform(
		_assembly,
		_analysisInfo,
		_julia,
		_svm15,
		_identifierAccess,
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
		julia::AbstractAssembly& _assembly,
		polynomial::assembly::AsmAnalysisInfo& _analysisInfo,
		bool _julia,
		bool _svm15,
		ExternalIdentifierAccess const& _identifierAccess,
		int _stackAdjustment,
		std::shared_ptr<Context> _context
	):
		m_assembly(_assembly),
		m_info(_analysisInfo),
		m_julia(_julia),
		m_svm15(_svm15),
		m_identifierAccess(_identifierAccess),
		m_stackAdjustment(_stackAdjustment),
		m_context(_context)
	{}

public:
	void operator()(polynomial::assembly::Instruction const& _instruction);
	void operator()(polynomial::assembly::Literal const& _literal);
	void operator()(polynomial::assembly::Identifier const& _identifier);
	void operator()(polynomial::assembly::FunctionalInstruction const& _instr);
	void operator()(polynomial::assembly::FunctionCall const&);
	void operator()(polynomial::assembly::Label const& _label);
	void operator()(polynomial::assembly::StackAssignment const& _assignment);
	void operator()(polynomial::assembly::Assignment const& _assignment);
	void operator()(polynomial::assembly::VariableDeclaration const& _varDecl);
	void operator()(polynomial::assembly::Switch const& _switch);
	void operator()(polynomial::assembly::FunctionDefinition const&);
	void operator()(polynomial::assembly::ForLoop const&);
	void operator()(polynomial::assembly::Block const& _block);

private:
	AbstractAssembly::LabelID labelFromIdentifier(polynomial::assembly::Identifier const& _identifier);
	/// @returns the label ID corresponding to the given label, allocating a new one if
	/// necessary.
	AbstractAssembly::LabelID labelID(polynomial::assembly::Scope::Label const& _label);
	AbstractAssembly::LabelID functionEntryID(polynomial::assembly::Scope::Function const& _function);
	/// Generates code for an expression that is supposed to return a single value.
	void visitExpression(polynomial::assembly::Statement const& _expression);

	void visitStatements(std::vector<polynomial::assembly::Statement> const& _statements);

	/// Pops all variables declared in the block and checks that the stack height is equal
	/// to @a _blackStartStackHeight.
	void finalizeBlock(polynomial::assembly::Block const& _block, int _blockStartStackHeight);

	void generateAssignment(polynomial::assembly::Identifier const& _variableName);

	/// Determines the stack height difference to the given variables. Throws
	/// if it is not yet in scope or the height difference is too large. Returns
	/// the (positive) stack height difference otherwise.
	int variableHeightDiff(polynomial::assembly::Scope::Variable const& _var, bool _forSwap);

	void expectDeposit(int _deposit, int _oldHeight);

	void checkStackHeight(void const* _astElement);

	julia::AbstractAssembly& m_assembly;
	polynomial::assembly::AsmAnalysisInfo& m_info;
	polynomial::assembly::Scope* m_scope = nullptr;
	bool m_julia = false;
	bool m_svm15 = false;
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
