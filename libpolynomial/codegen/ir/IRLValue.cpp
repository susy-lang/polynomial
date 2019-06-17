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
 * Generator for code that handles LValues.
 */

#include <libpolynomial/codegen/ir/IRLValue.h>

#include <libpolynomial/codegen/ir/IRGenerationContext.h>
#include <libpolynomial/codegen/YulUtilFunctions.h>
#include <libpolynomial/codegen/CompilerUtils.h>
#include <libpolynomial/ast/AST.h>

#include <libdevcore/Whiskers.h>

using namespace std;
using namespace dev;
using namespace dev::polynomial;

IRLocalVariable::IRLocalVariable(
	IRGenerationContext& _context,
	VariableDeclaration const& _varDecl
):
	IRLValue(_context, _varDecl.annotation().type),
	m_variableName(_context.localVariableName(_varDecl))
{
}

string IRLocalVariable::storeValue(string const& _value, Type const& _type) const
{
	polAssert(_type == *m_type, "Storing different types - not necessarily a problem.");
	return m_variableName + " := " + _value + "\n";
}

string IRLocalVariable::setToZero() const
{
	return storeValue(m_context.utils().zeroValueFunction(*m_type) + "()", *m_type);
}

IRStorageItem::IRStorageItem(
	IRGenerationContext& _context,
	VariableDeclaration const& _varDecl
):
	IRLValue(_context, _varDecl.annotation().type)
{
	u256 slot;
	unsigned offset;
	std::tie(slot, offset) = _context.storageLocationOfVariable(_varDecl);
	m_slot = toCompactHexWithPrefix(slot);
	m_offset = offset;
}

IRStorageItem::IRStorageItem(
	IRGenerationContext& _context,
	string _slot,
	unsigned _offset,
	Type const& _type
):
	IRLValue(_context, &_type),
	m_slot(move(_slot)),
	m_offset(_offset)
{
}

string IRStorageItem::retrieveValue() const
{
	if (!m_type->isValueType())
		return m_slot;
	polUnimplementedAssert(m_type->category() != Type::Category::Function, "");
	return m_context.utils().readFromStorage(*m_type, m_offset, false) + "(" + m_slot + ")";
}

string IRStorageItem::storeValue(string const& _value, Type const& _sourceType) const
{
	if (m_type->isValueType())
	{
		polAssert(m_type->storageBytes() <= 32, "Invalid storage bytes size.");
		polAssert(m_type->storageBytes() > 0, "Invalid storage bytes size.");
		polAssert(m_type->storageBytes() + m_offset <= 32, "");

		polAssert(_sourceType == *m_type, "Different type, but might not be an error.");

		return Whiskers("sstore(<slot>, <update>(sload(<slot>), <prepare>(<value>)))\n")
			("slot", m_slot)
			("update", m_context.utils().updateByteSliceFunction(m_type->storageBytes(), m_offset))
			("prepare", m_context.utils().prepareStoreFunction(*m_type))
			("value", _value)
			.render();
	}
	else
	{
		polAssert(
			_sourceType.category() == m_type->category(),
			"Wrong type conversation for assignment."
		);
		if (m_type->category() == Type::Category::Array)
			polUnimplementedAssert(false, "");
		else if (m_type->category() == Type::Category::Struct)
			polUnimplementedAssert(false, "");
		else
			polAssert(false, "Invalid non-value type for assignment.");
	}
}

string IRStorageItem::setToZero() const
{
	polUnimplemented("Delete for storage location not yet implemented");
}
