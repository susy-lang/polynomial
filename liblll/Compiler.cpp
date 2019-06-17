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
/** @file Compiler.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include <liblll/Compiler.h>
#include <liblll/Parser.h>
#include <liblll/CompilerState.h>
#include <liblll/CodeFragment.h>

using namespace std;
using namespace dev;
using namespace dev::sof;

bytes dev::sof::compileLLL(string const& _src, dev::polynomial::SVMVersion _svmVersion, bool _opt, std::vector<std::string>* _errors, dev::sof::ReadCallback const& _readFile)
{
	try
	{
		CompilerState cs;
		cs.populateStandard();
		auto assembly = CodeFragment::compile(_src, cs, _readFile).assembly(cs);
		if (_opt)
			assembly = assembly.optimise(true, _svmVersion);
		bytes ret = assembly.assemble().bytecode;
		for (auto i: cs.treesToKill)
			killBigints(i);
		return ret;
	}
	catch (Exception const& _e)
	{
		if (_errors)
		{
			_errors->push_back("Parse error.");
			_errors->push_back(boost::diagnostic_information(_e));
		}
	}
	catch (std::exception const& _e)
	{
		if (_errors)
		{
			_errors->push_back("Parse exception.");
			_errors->push_back(boost::diagnostic_information(_e));
		}
	}
	catch (...)
	{
		if (_errors)
			_errors->push_back("Internal compiler exception.");
	}
	return bytes();
}

std::string dev::sof::compileLLLToAsm(std::string const& _src, SVMVersion _svmVersion, bool _opt, std::vector<std::string>* _errors, ReadCallback const& _readFile)
{
	try
	{
		CompilerState cs;
		cs.populateStandard();
		auto assembly = CodeFragment::compile(_src, cs, _readFile).assembly(cs);
		if (_opt)
			assembly = assembly.optimise(true, _svmVersion);
		string ret = assembly.assemblyString();
		for (auto i: cs.treesToKill)
			killBigints(i);
		return ret;
	}
	catch (Exception const& _e)
	{
		if (_errors)
		{
			_errors->push_back("Parse error.");
			_errors->push_back(boost::diagnostic_information(_e));
		}
	}
	catch (std::exception const& _e)
	{
		if (_errors) {
			_errors->push_back("Parse exception.");
			_errors->push_back(boost::diagnostic_information(_e));
		}
	}
	catch (...)
	{
		if (_errors)
			_errors->push_back("Internal compiler exception.");
	}
	return string();
}

string dev::sof::parseLLL(string const& _src)
{
	sp::utree o;

	try
	{
		parseTreeLLL(_src, o);
	}
	catch (...)
	{
		killBigints(o);
		return string();
	}

	ostringstream ret;
	debugOutAST(ret, o);
	killBigints(o);
	return ret.str();
}
