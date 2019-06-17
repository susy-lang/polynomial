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

#include <string>
#include <functional>
#include <boost/noncopyable.hpp>

namespace dev
{

namespace polynomial
{

class ReadFile: boost::noncopyable
{
public:
	/// File reading result.
	struct Result
	{
		bool success;
		std::string contentsOrErrorMessage;
	};

	/// File reading callback.
	using Callback = std::function<Result(std::string const&)>;
};

}
}