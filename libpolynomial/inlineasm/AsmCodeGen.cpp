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
 * Code-generating part of inline assembly.
 */

#include <libpolynomial/inlineasm/AsmCodeGen.h>
#include <memory>
#include <functional>
#include <libsvmasm/Assembly.h>
#include <libsvmasm/SourceLocation.h>
#include <libsvmasm/Instruction.h>
#include <libpolynomial/inlineasm/AsmParser.h>
#include <libpolynomial/inlineasm/AsmData.h>

using namespace std;
using namespace dev;
using namespace dev::polynomial;
using namespace dev::polynomial::assembly;

struct GeneratorState
{
	explicit GeneratorState(ErrorList& _errors): errors(_errors) {}

	void addError(Error::Type _type, std::string const& _description, SourceLocation const& _location = SourceLocation())
	{
		auto err = make_shared<Error>(_type);
		if (!_location.isEmpty())
			*err << errinfo_sourceLocation(_location);
		*err << errinfo_comment(_description);
		errors.push_back(err);
	}

	int const* findVariable(string const& _variableName) const
	{
		auto localVariable = find_if(
			variables.rbegin(),
			variables.rend(),
			[&](pair<string, int> const& _var) { return _var.first == _variableName; }
		);
		return localVariable != variables.rend() ? &localVariable->second : nullptr;
	}
	sof::AssemblyItem const* findLabel(string const& _labelName) const
	{
		auto label = find_if(
			labels.begin(),
			labels.end(),
			[&](pair<string, sof::AssemblyItem> const& _label) { return _label.first == _labelName; }
		);
		return label != labels.end() ? &label->second : nullptr;
	}

	sof::Assembly assembly;
	map<string, sof::AssemblyItem> labels;
	vector<pair<string, int>> variables; ///< name plus stack height
	ErrorList& errors;
};

/**
 * Scans the inline assembly data for labels, creates tags in the assembly and searches for
 * duplicate labels.
 */
class LabelOrganizer: public boost::static_visitor<>
{
public:
	LabelOrganizer(GeneratorState& _state): m_state(_state) {}

	template <class T>
	void operator()(T const& /*_item*/) { }
	void operator()(Label const& _item)
	{
		if (m_state.labels.count(_item.name))
			//@TODO location and secondary location
			m_state.addError(Error::Type::DeclarationError, "Label " + _item.name + " declared twice.");
		m_state.labels.insert(make_pair(_item.name, m_state.assembly.newTag()));
	}
	void operator()(assembly::Block const& _block)
	{
		std::for_each(_block.statements.begin(), _block.statements.end(), boost::apply_visitor(*this));
	}

private:
	GeneratorState& m_state;
};

class CodeTransform: public boost::static_visitor<>
{
public:
	/// Create the code transformer which appends assembly to _state.assembly when called
	/// with parsed assembly data.
	/// @param _identifierAccess used to resolve identifiers external to the inline assembly
	explicit CodeTransform(
		GeneratorState& _state,
		assembly::CodeGenerator::IdentifierAccess const& _identifierAccess = assembly::CodeGenerator::IdentifierAccess()
	):
		m_state(_state)
	{
		if (_identifierAccess)
			m_identifierAccess = _identifierAccess;
		else
			m_identifierAccess = [](assembly::Identifier const&, sof::Assembly&, CodeGenerator::IdentifierContext) { return false; };
	}

	void operator()(dev::polynomial::assembly::Instruction const& _instruction)
	{
		m_state.assembly.append(_instruction.instruction);
	}
	void operator()(assembly::Literal const& _literal)
	{
		if (_literal.isNumber)
			m_state.assembly.append(u256(_literal.value));
		else if (_literal.value.size() > 32)
			m_state.addError(
				Error::Type::TypeError,
				"String literal too long (" + boost::lexical_cast<string>(_literal.value.size()) + " > 32)"
			);
		else
			m_state.assembly.append(_literal.value);
	}
	void operator()(assembly::Identifier const& _identifier)
	{
		// First search local variables, then labels, then externals.
		if (int const* stackHeight = m_state.findVariable(_identifier.name))
		{
			int heightDiff = m_state.assembly.deposit() - *stackHeight;
			if (heightDiff <= 0 || heightDiff > 16)
				//@TODO location
				m_state.addError(
					Error::Type::TypeError,
					"Variable inaccessible, too deep inside stack (" + boost::lexical_cast<string>(heightDiff) + ")"
				);
			else
				m_state.assembly.append(polynomial::dupInstruction(heightDiff));
			return;
		}
		else if (sof::AssemblyItem const* label = m_state.findLabel(_identifier.name))
			m_state.assembly.append(label->pushTag());
		else if (!m_identifierAccess(_identifier, m_state.assembly, CodeGenerator::IdentifierContext::RValue))
			m_state.addError(
				Error::Type::DeclarationError,
				"Identifier \"" + string(_identifier.name) + "\" not found or not unique"
			);
	}
	void operator()(FunctionalInstruction const& _instr)
	{
		for (auto it = _instr.arguments.rbegin(); it != _instr.arguments.rend(); ++it)
		{
			int height = m_state.assembly.deposit();
			boost::apply_visitor(*this, *it);
			expectDeposit(1, height);
		}
		(*this)(_instr.instruction);
	}
	void operator()(Label const& _label)
	{
		m_state.assembly.append(m_state.labels.at(_label.name));
	}
	void operator()(assembly::Assignment const& _assignment)
	{
		generateAssignment(_assignment.variableName);
	}
	void operator()(FunctionalAssignment const& _assignment)
	{
		int height = m_state.assembly.deposit();
		boost::apply_visitor(*this, *_assignment.value);
		expectDeposit(1, height);
		generateAssignment(_assignment.variableName);
	}
	void operator()(assembly::VariableDeclaration const& _varDecl)
	{
		int height = m_state.assembly.deposit();
		boost::apply_visitor(*this, *_varDecl.value);
		expectDeposit(1, height);
		m_state.variables.push_back(make_pair(_varDecl.name, height));
	}
	void operator()(assembly::Block const& _block)
	{
		size_t numVariables = m_state.variables.size();
		std::for_each(_block.statements.begin(), _block.statements.end(), boost::apply_visitor(*this));
		// pop variables
		//@TODO check height before and after
		while (m_state.variables.size() > numVariables)
		{
			m_state.assembly.append(polynomial::Instruction::POP);
			m_state.variables.pop_back();
		}
	}

private:
	void generateAssignment(assembly::Identifier const& _variableName)
	{
		if (int const* stackHeight = m_state.findVariable(_variableName.name))
		{
			int heightDiff = m_state.assembly.deposit() - *stackHeight - 1;
			if (heightDiff <= 0 || heightDiff > 16)
				//@TODO location
				m_state.addError(
					Error::Type::TypeError,
					"Variable inaccessible, too deep inside stack (" + boost::lexical_cast<string>(heightDiff) + ")"
				);
			else
			{
				m_state.assembly.append(polynomial::swapInstruction(heightDiff));
				m_state.assembly.append(polynomial::Instruction::POP);
			}
			return;
		}
		else if (!m_identifierAccess(_variableName, m_state.assembly, CodeGenerator::IdentifierContext::LValue))
			m_state.addError(
				Error::Type::DeclarationError,
				"Identifier \"" + string(_variableName.name) + "\" not found, not unique or not lvalue."
			);
	}

	void expectDeposit(int _deposit, int _oldHeight)
	{
		if (m_state.assembly.deposit() != _oldHeight + 1)
			//@TODO location
			m_state.addError(Error::Type::TypeError,
				"Expected instruction(s) to deposit " +
				boost::lexical_cast<string>(_deposit) +
				" item(s) to the stack, but did deposit " +
				boost::lexical_cast<string>(m_state.assembly.deposit() - _oldHeight) +
				" item(s)."
			);
	}

	GeneratorState& m_state;
	assembly::CodeGenerator::IdentifierAccess m_identifierAccess;
};

bool assembly::CodeGenerator::typeCheck(assembly::CodeGenerator::IdentifierAccess const& _identifierAccess)
{
	size_t initialErrorLen = m_errors.size();
	GeneratorState state(m_errors);
	(LabelOrganizer(state))(m_parsedData);
	(CodeTransform(state, _identifierAccess))(m_parsedData);
	return m_errors.size() == initialErrorLen;
}

sof::Assembly assembly::CodeGenerator::assemble(assembly::CodeGenerator::IdentifierAccess const& _identifierAccess)
{
	GeneratorState state(m_errors);
	(LabelOrganizer(state))(m_parsedData);
	(CodeTransform(state, _identifierAccess))(m_parsedData);
	return state.assembly;
}

