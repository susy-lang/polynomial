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
using namespace dev::polynomial;

smt::SortPointer dev::polynomial::smtSort(Type const& _type)
{
	switch (smtKind(_type.category()))
	{
	case smt::Kind::Int:
		return make_shared<smt::Sort>(smt::Kind::Int);
	case smt::Kind::Bool:
		return make_shared<smt::Sort>(smt::Kind::Bool);
	case smt::Kind::Function:
	{
		auto fType = dynamic_cast<FunctionType const*>(&_type);
		polAssert(fType, "");
		vector<smt::SortPointer> parameterSorts = smtSort(fType->parameterTypes());
		auto returnTypes = fType->returnParameterTypes();
		smt::SortPointer returnSort;
		// TODO change this when we support tuples.
		if (returnTypes.size() == 0)
			// We cannot declare functions without a return sort, so we use the smallest.
			returnSort = make_shared<smt::Sort>(smt::Kind::Bool);
		else if (returnTypes.size() > 1)
			// Abstract sort.
			returnSort = make_shared<smt::Sort>(smt::Kind::Int);
		else
			returnSort = smtSort(*returnTypes.at(0));
		return make_shared<smt::FunctionSort>(parameterSorts, returnSort);
	}
	case smt::Kind::Array:
	{
		if (isMapping(_type.category()))
		{
			auto mapType = dynamic_cast<MappingType const*>(&_type);
			polAssert(mapType, "");
			return make_shared<smt::ArraySort>(smtSort(*mapType->keyType()), smtSort(*mapType->valueType()));
		}
		else
		{
			polAssert(isArray(_type.category()), "");
			auto arrayType = dynamic_cast<ArrayType const*>(&_type);
			polAssert(arrayType, "");
			return make_shared<smt::ArraySort>(make_shared<smt::Sort>(smt::Kind::Int), smtSort(*arrayType->baseType()));
		}
	}
	default:
		// Abstract case.
		return make_shared<smt::Sort>(smt::Kind::Int);
	}
}

vector<smt::SortPointer> dev::polynomial::smtSort(vector<TypePointer> const& _types)
{
	vector<smt::SortPointer> sorts;
	for (auto const& type: _types)
		sorts.push_back(smtSort(*type));
	return sorts;
}

smt::Kind dev::polynomial::smtKind(Type::Category _category)
{
	if (isNumber(_category))
		return smt::Kind::Int;
	else if (isBool(_category))
		return smt::Kind::Bool;
	else if (isFunction(_category))
		return smt::Kind::Function;
	else if (isMapping(_category) || isArray(_category))
		return smt::Kind::Array;
	// Abstract case.
	return smt::Kind::Int;
}

bool dev::polynomial::isSupportedType(Type::Category _category)
{
	return isNumber(_category) ||
		isBool(_category) ||
		isMapping(_category) ||
		isArray(_category);
}

bool dev::polynomial::isSupportedTypeDeclaration(Type::Category _category)
{
	return isSupportedType(_category) ||
		isFunction(_category);
}

pair<bool, shared_ptr<SymbolicVariable>> dev::polynomial::newSymbolicVariable(
	Type const& _type,
	std::string const& _uniqueName,
	smt::SolverInterface& _solver
)
{
	bool abstract = false;
	shared_ptr<SymbolicVariable> var;
	TypePointer type = &_type;
	if (!isSupportedTypeDeclaration(_type))
	{
		abstract = true;
		var = make_shared<SymbolicIntVariable>(TypeProvider::uint256(), _uniqueName, _solver);
	}
	else if (isBool(_type.category()))
		var = make_shared<SymbolicBoolVariable>(type, _uniqueName, _solver);
	else if (isFunction(_type.category()))
		var = make_shared<SymbolicFunctionVariable>(type, _uniqueName, _solver);
	else if (isInteger(_type.category()))
		var = make_shared<SymbolicIntVariable>(type, _uniqueName, _solver);
	else if (isFixedBytes(_type.category()))
	{
		auto fixedBytesType = dynamic_cast<FixedBytesType const*>(type);
		polAssert(fixedBytesType, "");
		var = make_shared<SymbolicFixedBytesVariable>(fixedBytesType->numBytes(), _uniqueName, _solver);
	}
	else if (isAddress(_type.category()) || isContract(_type.category()))
		var = make_shared<SymbolicAddressVariable>(_uniqueName, _solver);
	else if (isEnum(_type.category()))
		var = make_shared<SymbolicEnumVariable>(type, _uniqueName, _solver);
	else if (isRational(_type.category()))
	{
		auto rational = dynamic_cast<RationalNumberType const*>(&_type);
		polAssert(rational, "");
		if (rational->isFractional())
			var = make_shared<SymbolicIntVariable>(TypeProvider::uint256(), _uniqueName, _solver);
		else
			var = make_shared<SymbolicIntVariable>(type, _uniqueName, _solver);
	}
	else if (isMapping(_type.category()))
		var = make_shared<SymbolicMappingVariable>(type, _uniqueName, _solver);
	else if (isArray(_type.category()))
		var = make_shared<SymbolicArrayVariable>(type, _uniqueName, _solver);
	else
		polAssert(false, "");
	return make_pair(abstract, var);
}

bool dev::polynomial::isSupportedType(Type const& _type)
{
	return isSupportedType(_type.category());
}

bool dev::polynomial::isSupportedTypeDeclaration(Type const& _type)
{
	return isSupportedTypeDeclaration(_type.category());
}

bool dev::polynomial::isInteger(Type::Category _category)
{
	return _category == Type::Category::Integer;
}

bool dev::polynomial::isRational(Type::Category _category)
{
	return _category == Type::Category::RationalNumber;
}

bool dev::polynomial::isFixedBytes(Type::Category _category)
{
	return _category == Type::Category::FixedBytes;
}

bool dev::polynomial::isAddress(Type::Category _category)
{
	return _category == Type::Category::Address;
}

bool dev::polynomial::isContract(Type::Category _category)
{
	return _category == Type::Category::Contract;
}

bool dev::polynomial::isEnum(Type::Category _category)
{
	return _category == Type::Category::Enum;
}

bool dev::polynomial::isNumber(Type::Category _category)
{
	return isInteger(_category) ||
		isRational(_category) ||
		isFixedBytes(_category) ||
		isAddress(_category) ||
		isContract(_category) ||
		isEnum(_category);
}

bool dev::polynomial::isBool(Type::Category _category)
{
	return _category == Type::Category::Bool;
}

bool dev::polynomial::isFunction(Type::Category _category)
{
	return _category == Type::Category::Function;
}

bool dev::polynomial::isMapping(Type::Category _category)
{
	return _category == Type::Category::Mapping;
}

bool dev::polynomial::isArray(Type::Category _category)
{
	return _category == Type::Category::Array;
}

smt::Expression dev::polynomial::minValue(IntegerType const& _type)
{
	return smt::Expression(_type.minValue());
}

smt::Expression dev::polynomial::maxValue(IntegerType const& _type)
{
	return smt::Expression(_type.maxValue());
}

void dev::polynomial::smt::setSymbolicZeroValue(SymbolicVariable const& _variable, smt::SolverInterface& _interface)
{
	setSymbolicZeroValue(_variable.currentValue(), _variable.type(), _interface);
}

void dev::polynomial::smt::setSymbolicZeroValue(smt::Expression _expr, TypePointer const& _type, smt::SolverInterface& _interface)
{
	polAssert(_type, "");
	if (isInteger(_type->category()))
		_interface.addAssertion(_expr == 0);
	else if (isBool(_type->category()))
		_interface.addAssertion(_expr == smt::Expression(false));
}

void dev::polynomial::smt::setSymbolicUnknownValue(SymbolicVariable const& _variable, smt::SolverInterface& _interface)
{
	setSymbolicUnknownValue(_variable.currentValue(), _variable.type(), _interface);
}

void dev::polynomial::smt::setSymbolicUnknownValue(smt::Expression _expr, TypePointer const& _type, smt::SolverInterface& _interface)
{
	polAssert(_type, "");
	if (isEnum(_type->category()))
	{
		auto enumType = dynamic_cast<EnumType const*>(_type);
		polAssert(enumType, "");
		_interface.addAssertion(_expr >= 0);
		_interface.addAssertion(_expr < enumType->numberOfMembers());
	}
	else if (isInteger(_type->category()))
	{
		auto intType = dynamic_cast<IntegerType const*>(_type);
		polAssert(intType, "");
		_interface.addAssertion(_expr >= minValue(*intType));
		_interface.addAssertion(_expr <= maxValue(*intType));
	}
}
