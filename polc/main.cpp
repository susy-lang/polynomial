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
 * Polynomial commandline compiler.
 */

#include "CommandLineInterface.h"
#include <iostream>
#include <boost/exception/all.hpp>

using namespace std;

int main(int argc, char** argv)
{
	dev::polynomial::CommandLineInterface cli;
	if (!cli.parseArguments(argc, argv))
		return 1;
	if (!cli.processInput())
		return 1;
	try
	{
		cli.actOnInput();
	}
	catch (boost::exception const& _exception)
	{
		cerr << "Exception during output generation: " << boost::diagnostic_information(_exception) << endl;
		return 1;
	}

	return 0;
}
