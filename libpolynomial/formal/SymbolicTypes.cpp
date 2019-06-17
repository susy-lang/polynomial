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

#include <libpolynomial/formal/SymbolicTypes.h>

#include <libpolynomial/ast/TypeProvider.h>
#include <libpolynomial/ast/Types.h>
#include <memory>

using namespace std;

namespace dev
{
namespace polynomial
{
namespace smt
{

SortPointer smtSort(polynomial::Type const& _type)
{
	switch (smtKind(_type.category()))
	{
	case Kind::Int:
		return make_shared<Sort>(Kind::Int);
	case Kind::Bool:
		return make_shared<Sort>(Kind::Bool);
	case Kind::Function:
	{
		auto fType = dynamic_cast<polynomial::FunctionType const*>(&_type);
		polAssert(fType, "");
		vector<SortPointer> parameterSorts = smtSort(fType->parameterTypes());
		auto returnTypes = fType->returnParameterTypes();
		SortPointer returnSort;
		// TODO change this when we support tuples.
		if (returnTypes.size() == 0)
			// We cannot declare functions without a return sort, so we use the smallest.
			returnSort = make_shared<Sort>(Kind::Bool);
		else if (returnTypes.size() > 1)
			// Abstract sort.
			returnSort = make_shared<Sort>(Kind::Int);
		else
			returnSort = smtSort(*returnTypes.front());
		return make_shared<FunctionSort>(parameterSorts, returnSort);
	}
	case Kind::Array:
	{
		if (isMapping(_type.category()))
		{
			auto mapType = dynamic_cast<polynomial::MappingType const*>(&_type);
			polAssert(mapType, "");
			return make_shared<ArraySort>(smtSort(*mapType->keyType()), smtSort(*mapType->valueType()));
		}
		else
		{
			polAssert(isArray(_type.category()), "");
			auto arrayType = dynamic_cast<polynomial::ArrayType const*>(&_type);
			polAssert(arrayType, "");
			return make_shared<ArraySort>(make_shared<Sort>(Kind::Int), smtSort(*arrayType->baseType()));
		}
	}
	default:
		// Abstract case.
		return make_shared<Sort>(Kind::Int);
	}
}

vector<SortPointer> smtSort(vector<polynomial::TypePointer> const& _types)
{
	vector<SortPointer> sorts;
	for (auto const& type: _types)
		sorts.push_back(smtSort(*type));
	return sorts;
}

Kind smtKind(polynomial::Type::Category _category)
{
	if (isNumber(_category))
		return Kind::Int;
	else if (isBool(_category))
		return Kind::Bool;
	else if (isFunction(_category))
		return Kind::Function;
	else if (isMapping(_category) || isArray(_category))
		return Kind::Array;
	// Abstract case.
	return Kind::Int;
}

bool isSupportedType(polynomial::Type::Category _category)
{
	return isNumber(_category) ||
		isBool(_category) ||
		isMapping(_category) ||
		isArray(_category) ||
		isTuple(_category);
}

bool isSupportedTypeDeclaration(polynomial::Type::Category _category)
{
	return isSupportedType(_category) ||
		isFunction(_category);
}

pair<bool, shared_ptr<SymbolicVariable>> newSymbolicVariable(
	polynomial::Type const& _type,
	std::string const& _uniqueName,
	SolverInterface& _solver
)
{
	bool abstract = false;
	shared_ptr<SymbolicVariable> var;
	polynomial::TypePointer type = &_type;
	if (!isSupportedTypeDeclaration(_type))
	{
		abstract = true;
		var = make_shared<SymbolicIntVariable>(polynomial::TypeProvider::uint256(), _uniqueName, _solver);
	}
	else if (isBool(_type.category()))
		var = make_shared<SymbolicBoolVariable>(type, _uniqueName, _solver);
	else if (isFunction(_type.category()))
		var = make_shared<SymbolicFunctionVariable>(type, _uniqueName, _solver);
	else if (isInteger(_type.category()))
		var = make_shared<SymbolicIntVariable>(type, _uniqueName, _solver);
	else if (isFixedBytes(_type.category()))
	{
		auto fixedBytesType = dynamic_cast<polynomial::FixedBytesType const*>(type);
		polAssert(fixedBytesType, "");
		var = make_shared<SymbolicFixedBytesVariable>(fixedBytesType->numBytes(), _uniqueName, _solver);
	}
	else if (isAddress(_type.category()) || isContract(_type.category()))
		var = make_shared<SymbolicAddressVariable>(_uniqueName, _solver);
	else if (isEnum(_type.category()))
		var = make_shared<SymbolicEnumVariable>(type, _uniqueName, _solver);
	else if (isRational(_type.category()))
	{
		auto rational = dynamic_cast<polynomial::RationalNumberType const*>(&_type);
		polAssert(rational, "");
		if (rational->isFractional())
			var = make_shared<SymbolicIntVariable>(polynomial::TypeProvider::uint256(), _uniqueName, _solver);
		else
			var = make_shared<SymbolicIntVariable>(type, _uniqueName, _solver);
	}
	else if (isMapping(_type.category()))
		var = make_shared<SymbolicMappingVariable>(type, _uniqueName, _solver);
	else if (isArray(_type.category()))
		var = make_shared<SymbolicArrayVariable>(type, _uniqueName, _solver);
	else if (isTuple(_type.category()))
		var = make_shared<SymbolicTupleVariable>(type, _uniqueName, _solver);
	else
		polAssert(false, "");
	return make_pair(abstract, var);
}

bool isSupportedType(polynomial::Type const& _type)
{
	return isSupportedType(_type.category());
}

bool isSupportedTypeDeclaration(polynomial::Type const& _type)
{
	return isSupportedTypeDeclaration(_type.category());
}

bool isInteger(polynomial::Type::Category _category)
{
	return _category == polynomial::Type::Category::Integer;
}

bool isRational(polynomial::Type::Category _category)
{
	return _category == polynomial::Type::Category::RationalNumber;
}

bool isFixedBytes(polynomial::Type::Category _category)
{
	return _category == polynomial::Type::Category::FixedBytes;
}

bool isAddress(polynomial::Type::Category _category)
{
	return _category == polynomial::Type::Category::Address;
}

bool isContract(polynomial::Type::Category _category)
{
	return _category == polynomial::Type::Category::Contract;
}

bool isEnum(polynomial::Type::Category _category)
{
	return _category == polynomial::Type::Category::Enum;
}

bool isNumber(polynomial::Type::Category _category)
{
	return isInteger(_category) ||
		isRational(_category) ||
		isFixedBytes(_category) ||
		isAddress(_category) ||
		isContract(_category) ||
		isEnum(_category);
}

bool isBool(polynomial::Type::Category _category)
{
	return _category == polynomial::Type::Category::Bool;
}

bool isFunction(polynomial::Type::Category _category)
{
	return _category == polynomial::Type::Category::Function;
}

bool isMapping(polynomial::Type::Category _category)
{
	return _category == polynomial::Type::Category::Mapping;
}

bool isArray(polynomial::Type::Category _category)
{
	return _category == polynomial::Type::Category::Array;
}

bool isTuple(polynomial::Type::Category _category)
{
	return _category == polynomial::Type::Category::Tuple;
}

Expression minValue(polynomial::IntegerType const& _type)
{
	return Expression(_type.minValue());
}

Expression maxValue(polynomial::IntegerType const& _type)
{
	return Expression(_type.maxValue());
}

void setSymbolicZeroValue(SymbolicVariable const& _variable, SolverInterface& _interface)
{
	setSymbolicZeroValue(_variable.currentValue(), _variable.type(), _interface);
}

void setSymbolicZeroValue(Expression _expr, polynomial::TypePointer const& _type, SolverInterface& _interface)
{
	polAssert(_type, "");
	if (isInteger(_type->category()))
		_interface.addAssertion(_expr == 0);
	else if (isBool(_type->category()))
		_interface.addAssertion(_expr == Expression(false));
}

void setSymbolicUnknownValue(SymbolicVariable const& _variable, SolverInterface& _interface)
{
	setSymbolicUnknownValue(_variable.currentValue(), _variable.type(), _interface);
}

void setSymbolicUnknownValue(Expression _expr, polynomial::TypePointer const& _type, SolverInterface& _interface)
{
	polAssert(_type, "");
	if (isEnum(_type->category()))
	{
		auto enumType = dynamic_cast<polynomial::EnumType const*>(_type);
		polAssert(enumType, "");
		_interface.addAssertion(_expr >= 0);
		_interface.addAssertion(_expr < enumType->numberOfMembers());
	}
	else if (isInteger(_type->category()))
	{
		auto intType = dynamic_cast<polynomial::IntegerType const*>(_type);
		polAssert(intType, "");
		_interface.addAssertion(_expr >= minValue(*intType));
		_interface.addAssertion(_expr <= maxValue(*intType));
	}
}

}
}
}
