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

#pragma once

#include <boost/noncopyable.hpp>
#include <functional>
#include <string>

namespace dev
{

namespace polynomial
{

class ReadCallback: boost::noncopyable
{
public:
	/// File reading or generic query result.
	struct Result
	{
		bool success;
		std::string responseOrErrorMessage;
	};

	/// File reading or generic query callback.
	using Callback = std::function<Result(std::string const&)>;
};

}
}
