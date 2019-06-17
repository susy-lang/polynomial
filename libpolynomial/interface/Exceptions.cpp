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
 * @author Liana <liana@sofdev.com>
 * @date 2015
 * Polynomial exception hierarchy.
 */

#include <libpolynomial/interface/Exceptions.h>

using namespace std;
using namespace dev;
using namespace dev::polynomial;

Error::Error(Type _type, SourceLocation const& _location, string const& _description):
	m_type(_type)
{
	switch(m_type)
	{
	case Type::DeclarationError:
		m_typeName = "DeclarationError";
		break;
	case Type::DocstringParsingError:
		m_typeName = "DocstringParsingError";
		break;
	case Type::ParserError:
		m_typeName = "ParserError";
		break;
	case Type::SyntaxError:
		m_typeName = "SyntaxError";
		break;
	case Type::TypeError:
		m_typeName = "TypeError";
		break;
	case Type::Warning:
		m_typeName = "Warning";
		break;
	default:
		polAssert(false, "");
		break;
	}

	if (!_location.isEmpty())
		*this << errinfo_sourceLocation(_location);
	if (!_description.empty())
		*this << errinfo_comment(_description);
}

Error::Error(Error::Type _type, const std::string& _description, const SourceLocation& _location):
	Error(_type)
{
	if (!_location.isEmpty())
		*this << errinfo_sourceLocation(_location);
	*this << errinfo_comment(_description);
}

string Exception::lineInfo() const
{
	char const* const* file = boost::get_error_info<boost::throw_file>(*this);
	int const* line = boost::get_error_info<boost::throw_line>(*this);
	string ret;
	if (file)
		ret += *file;
	ret += ':';
	if (line)
		ret += boost::lexical_cast<string>(*line);
	return ret;
}
