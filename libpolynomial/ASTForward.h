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
 * @date 2014
 * Forward-declarations of AST classes.
 */

#pragma once

#include <string>
#include <memory>
#include <vector>

// Forward-declare all AST node types

namespace dev
{
namespace polynomial
{

class ASTNode;
class SourceUnit;
class ImportDirective;
class Declaration;
class ContractDefinition;
class InheritanceSpecifier;
class StructDefinition;
class EnumDefinition;
class EnumValue;
class ParameterList;
class FunctionDefinition;
class VariableDeclaration;
class ModifierDefinition;
class ModifierInvocation;
class EventDefinition;
class MagicVariableDeclaration;
class TypeName;
class ElementaryTypeName;
class UserDefinedTypeName;
class Mapping;
class ArrayTypeName;
class Statement;
class Block;
class PlaceholderStatement;
class IfStatement;
class BreakableStatement;
class WhileStatement;
class ForStatement;
class Continue;
class Break;
class Return;
class VariableDeclarationStatement;
class ExpressionStatement;
class Expression;
class Assignment;
class UnaryOperation;
class BinaryOperation;
class FunctionCall;
class NewExpression;
class MemberAccess;
class IndexAccess;
class PrimaryExpression;
class Identifier;
class ElementaryTypeNameExpression;
class Literal;

class VariableScope;

// Used as pointers to AST nodes, to be replaced by more clever pointers, e.g. pointers which do
// not do reference counting but point to a special memory area that is completely released
// explicitly.
template <class T>
using ASTPointer = std::shared_ptr<T>;

using ASTString = std::string;


}
}
