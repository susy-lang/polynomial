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
 * Formatting functions for errors referencing positions and locations in the source.
 */

#include <libpolynomial/interface/SourceReferenceFormatter.h>
#include <libpolynomial/parsing/Scanner.h>
#include <libpolynomial/interface/Exceptions.h>

using namespace std;

namespace dev
{
namespace polynomial
{

void SourceReferenceFormatter::printSourceLocation(
	ostream& _stream,
	SourceLocation const& _location,
	Scanner const& _scanner
)
{
	int startLine;
	int startColumn;
	tie(startLine, startColumn) = _scanner.translatePositionToLineColumn(_location.start);
	int endLine;
	int endColumn;
	tie(endLine, endColumn) = _scanner.translatePositionToLineColumn(_location.end);
	if (startLine == endLine)
	{
		string line = _scanner.lineAtPosition(_location.start);
		_stream << line << endl;
		for_each(
			line.cbegin(),
			line.cbegin() + startColumn,
			[&_stream](char const& ch) { _stream << (ch == '\t' ? '\t' : ' '); }
		);
		_stream << "^";
		if (endColumn > startColumn + 2)
			_stream << string(endColumn - startColumn - 2, '-');
		if (endColumn > startColumn + 1)
			_stream << "^";
		_stream << endl;
	}
	else
		_stream <<
			_scanner.lineAtPosition(_location.start) <<
			endl <<
			string(startColumn, ' ') <<
			"^\n" <<
			"Spanning multiple lines.\n";
}

void SourceReferenceFormatter::printSourceName(
	ostream& _stream,
	SourceLocation const& _location,
	Scanner const& _scanner
)
{
	int startLine;
	int startColumn;
	tie(startLine, startColumn) = _scanner.translatePositionToLineColumn(_location.start);
	_stream << *_location.sourceName << ":" << (startLine + 1) << ":" << (startColumn + 1) << ": ";
}

void SourceReferenceFormatter::printExceptionInformation(
	ostream& _stream,
	Exception const& _exception,
	string const& _name,
	function<Scanner const&(string const&)> const& _scannerFromSourceName
)
{
	SourceLocation const* location = boost::get_error_info<errinfo_sourceLocation>(_exception);
	auto secondarylocation = boost::get_error_info<errinfo_secondarySourceLocation>(_exception);
	Scanner const* scannerPtr = nullptr;

	if (location && location->sourceName)
	{
		scannerPtr = &_scannerFromSourceName(*location->sourceName);
		printSourceName(_stream, *location, *scannerPtr);
	}

	_stream << _name;
	if (string const* description = boost::get_error_info<errinfo_comment>(_exception))
		_stream << ": " << *description << endl;

	if (location && location->sourceName)
	{
		scannerPtr = &_scannerFromSourceName(*location->sourceName);
		printSourceLocation(_stream, *location, *scannerPtr);
	}

	if (secondarylocation && !secondarylocation->infos.empty())
	{
		for (auto info: secondarylocation->infos)
		{
			_stream << info.first << " ";
			if (!info.second.sourceName)
			{
				_stream << endl;
				continue;
			}
			scannerPtr = &_scannerFromSourceName(*info.second.sourceName);
			printSourceName(_stream, info.second, *scannerPtr);
			_stream << endl;
			printSourceLocation(_stream, info.second, *scannerPtr);
		}
		_stream << endl;
	}
}

}
}
