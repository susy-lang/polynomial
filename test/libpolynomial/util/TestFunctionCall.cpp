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

#include <test/libpolynomial/util/TestFunctionCall.h>

#include <libdevcore/AnsiColorized.h>

#include <boost/algorithm/string/replace.hpp>

#include <regex>
#include <stdexcept>
#include <string>

using namespace dev;
using namespace polynomial;
using namespace dev::polynomial::test;
using namespace std;

namespace
{

static regex s_boolType{"(bool)"};
static regex s_uintType{"(uint\\d*)"};
static regex s_intType{"(int\\d*)"};
static regex s_bytesType{"(bytes\\d+)"};
static regex s_dynBytesType{"(\\bbytes\\b)"};
static regex s_stringType{"(string)"};

/// Translates Polynomial's ABI types into the internal type representation of
/// poltest.
auto contractABITypes(string const& _type) -> vector<ABIType>
{
	vector<ABIType> abiTypes;
	if (regex_match(_type, s_boolType))
		abiTypes.push_back(ABIType{ABIType::Boolean, ABIType::AlignRight, 32});
	else if (regex_match(_type, s_uintType))
		abiTypes.push_back(ABIType{ABIType::UnsignedDec, ABIType::AlignRight, 32});
	else if (regex_match(_type, s_intType))
		abiTypes.push_back(ABIType{ABIType::SignedDec, ABIType::AlignRight, 32});
	else if (regex_match(_type, s_bytesType))
		abiTypes.push_back(ABIType{ABIType::Hex, ABIType::AlignRight, 32});
	else if (regex_match(_type, s_dynBytesType))
	{
		abiTypes.push_back(ABIType{ABIType::UnsignedDec, ABIType::AlignRight, 32});
		abiTypes.push_back(ABIType{ABIType::UnsignedDec, ABIType::AlignRight, 32});
		abiTypes.push_back(ABIType{ABIType::HexString, ABIType::AlignLeft, 32});
	}
	else if (regex_match(_type, s_stringType))
	{
		abiTypes.push_back(ABIType{ABIType::UnsignedDec, ABIType::AlignRight, 32});
		abiTypes.push_back(ABIType{ABIType::UnsignedDec, ABIType::AlignRight, 32});
		abiTypes.push_back(ABIType{ABIType::String, ABIType::AlignLeft, 32});
	}
	else
		abiTypes.push_back(ABIType{ABIType::None, ABIType::AlignRight, 0});
	return abiTypes;
};
}

string TestFunctionCall::format(
	ErrorReporter& _errorReporter,
	string const& _linePrefix,
	bool const _renderResult,
	bool const _highlight
) const
{
	using namespace poltest;
	using Token = poltest::Token;

	stringstream stream;

	bool highlight = !matchesExpectation() && _highlight;

	auto formatOutput = [&](bool const _singleLine)
	{
		string ws = " ";
		string arrow = formatToken(Token::Arrow);
		string colon = formatToken(Token::Colon);
		string comma = formatToken(Token::Comma);
		string comment = formatToken(Token::Comment);
		string sophy = formatToken(Token::Sophy);
		string newline = formatToken(Token::Newline);
		string failure = formatToken(Token::Failure);

		/// Formats the function signature. This is the same independent from the display-mode.
		stream << _linePrefix << newline << ws << m_call.signature;
		if (m_call.value > u256(0))
			stream << comma << ws << m_call.value << ws << sophy;
		if (!m_call.arguments.rawBytes().empty())
		{
			string output = formatRawParameters(m_call.arguments.parameters, _linePrefix);
			stream << colon;
			if (_singleLine)
				stream << ws;
			stream << output;

		}

		/// Formats comments on the function parameters and the arrow taking
		/// the display-mode into account.
		if (_singleLine)
		{
			if (!m_call.arguments.comment.empty())
				stream << ws << comment << m_call.arguments.comment << comment;
			stream << ws << arrow << ws;
		}
		else
		{
			stream << endl << _linePrefix << newline << ws;
			if (!m_call.arguments.comment.empty())
			{
				 stream << comment << m_call.arguments.comment << comment;
				 stream << endl << _linePrefix << newline << ws;
			}
			stream << arrow << ws;
		}

		/// Format either the expected output or the actual result output
		string result;
		if (!_renderResult)
		{
			bytes output = m_call.expectations.rawBytes();
			bool const isFailure = m_call.expectations.failure;
			result = isFailure ?
				failure :
				formatRawParameters(m_call.expectations.result);
			AnsiColorized(stream, highlight, {dev::formatting::RED_BACKGROUND}) << result;
		}
		else
		{
			bytes output = m_rawBytes;
			bool const isFailure = m_failure;
			result = isFailure ?
				failure :
				matchesExpectation() ?
					formatRawParameters(m_call.expectations.result) :
					formatBytesParameters(
						_errorReporter,
						output,
						m_call.signature,
						m_call.expectations.result,
						highlight
					);

			if (isFailure)
				AnsiColorized(stream, highlight, {dev::formatting::RED_BACKGROUND}) << result;
			else
				stream << result;
		}

		/// Format comments on expectations taking the display-mode into account.
		if (_singleLine)
		{
			if (!m_call.expectations.comment.empty())
				stream << ws << comment << m_call.expectations.comment << comment;
		}
		else
		{
			if (!m_call.expectations.comment.empty())
			{
				stream << endl << _linePrefix << newline << ws;
				stream << comment << m_call.expectations.comment << comment;
			}
		}
	};

	formatOutput(m_call.displayMode == FunctionCall::DisplayMode::SingleLine);
	return stream.str();
}

string TestFunctionCall::formatBytesParameters(
	ErrorReporter& _errorReporter,
	bytes const& _bytes,
	string const& _signature,
	dev::polynomial::test::ParameterList const& _params,
	bool _highlight
) const
{
	using ParameterList = dev::polynomial::test::ParameterList;

	stringstream os;
	string functionName{_signature.substr(0, _signature.find("("))};

	auto sizeFold = [](size_t const _a, Parameter const& _b) { return _a + _b.abiType.size; };
	size_t encodingSize = std::accumulate(_params.begin(), _params.end(), size_t{0}, sizeFold);

	/// Infer type from Contract ABI. Used to generate values for
	/// auto-correction during interactive update routine.
	ParameterList abiParams;
	for (auto const& function: m_contractABI)
		if (function["name"] == functionName)
			for (auto const& output: function["outputs"])
			{
				auto types = contractABITypes(output["type"].asString());
				for (auto const& type: types)
					abiParams.push_back(Parameter{bytes(), "", type, FormatInfo{}});
			}


	/// If parameter count does not match, take types defined by ABI, but only
	/// if the contract ABI is defined (needed for format tests where the actual
	/// result does not matter).
	ParameterList preferredParams;
	if (m_contractABI && (_params.size() != abiParams.size()))
	{
		_errorReporter.warning(
			"Encoding does not match byte range. The call returned " +
			to_string(_bytes.size()) + " bytes, but " +
			to_string(encodingSize) + " bytes were expected."
		);
		preferredParams = abiParams;
	}
	else
		preferredParams = _params;

	/// If output is empty, do not format anything.
	if (_bytes.empty())
		return {};

	/// Format output bytes with the given parameters. ABI type takes precedence if:
	/// - size of ABI type is greater
	/// - given expected type does not match and needs to be overridden in order
	///   to generate a valid output of the parameter
	auto it = _bytes.begin();
	auto abiParam = abiParams.begin();
	size_t paramIndex = 1;
	for (auto const& param: preferredParams)
	{
		size_t size = param.abiType.size;
		if (m_contractABI)
			size = std::max((*abiParam).abiType.size, param.abiType.size);

		long offset = static_cast<long>(size);
		auto offsetIter = it + offset;
		bytes byteRange{it, offsetIter};

		/// Override type with ABI type if given one does not match.
		auto type = param.abiType;
		if (m_contractABI)
			if ((*abiParam).abiType.type > param.abiType.type)
			{
				type = (*abiParam).abiType;
				_errorReporter.warning(
					"Type of parameter " + to_string(paramIndex) +
					" does not match the one inferred from ABI."
				);
			}

		/// Prints obtained result if it does not match the expectation
		/// and prints the expected result otherwise.
		/// Highlights parameter only if it does not match.
		if (byteRange != param.rawBytes)
			AnsiColorized(
				os,
				_highlight,
				{dev::formatting::RED_BACKGROUND}
			) << formatBytesRange(byteRange, type);
		else
			os << param.rawString;

		if (abiParam != abiParams.end())
			abiParam++;

		it += offset;
		paramIndex++;
		if (&param != &preferredParams.back())
			os << ", ";
	}
	return os.str();
}

string TestFunctionCall::formatBytesRange(
	bytes const& _bytes,
	ABIType const& _abiType
) const
{
	stringstream os;

	switch (_abiType.type)
	{
	case ABIType::UnsignedDec:
		// Check if the detected type was wrong and if this could
		// be signed. If an unsigned was detected in the expectations,
		// but the actual result returned a signed, it would be formatted
		// incorrectly.
		if (*_bytes.begin() & 0x80)
			os << u2s(fromBigEndian<u256>(_bytes));
		else
			os << fromBigEndian<u256>(_bytes);
		break;
	case ABIType::SignedDec:
		if (*_bytes.begin() & 0x80)
			os << u2s(fromBigEndian<u256>(_bytes));
		else
			os << fromBigEndian<u256>(_bytes);
		break;
	case ABIType::Boolean:
	{
		u256 result = fromBigEndian<u256>(_bytes);
		if (result == 0)
			os << "false";
		else if (result == 1)
			os << "true";
		else
			os << result;
		break;
	}
	case ABIType::Hex:
	{
		string hex{toHex(_bytes, HexPrefix::Add)};
		boost::algorithm::replace_all(hex, "00", "");
		os << hex;
		break;
	}
	case ABIType::HexString:
		os << "hex\"" << toHex(_bytes) << "\"";
		break;
	case ABIType::String:
	{
		os << "\"";
		bool expectZeros = false;
		for (auto const& v: _bytes)
		{
			if (expectZeros && v != 0)
				return {};
			if (v == 0) expectZeros = true;
			else
			{
				if (!isprint(v) || v == '"')
					return {};
				os << v;
			}
		}
		os << "\"";
		break;
	}
	case ABIType::Failure:
		break;
	case ABIType::None:
		break;
	}
	return os.str();
}

string TestFunctionCall::formatRawParameters(
	dev::polynomial::test::ParameterList const& _params,
	std::string const& _linePrefix
) const
{
	stringstream os;
	for (auto const& param: _params)
	{
		if (param.format.newline)
			os << endl << _linePrefix << "// ";
		os << param.rawString;
		if (&param != &_params.back())
			os << ", ";
	}
	return os.str();
}

void TestFunctionCall::reset()
{
	m_rawBytes = bytes{};
	m_failure = true;
}

bool TestFunctionCall::matchesExpectation() const
{
	return m_failure == m_call.expectations.failure && m_rawBytes == m_call.expectations.rawBytes();
}
