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
/** @file Exceptions.h
 * @author Christian <c@sofdev.com>
 * @date 2014
 */

#pragma once

#include <libdevcore/Exceptions.h>

namespace dev
{
namespace sof
{

struct AssemblyException: virtual Exception {};
struct OptimizerException: virtual AssemblyException {};
struct StackTooDeepException: virtual OptimizerException {};
struct ItemNotAvailableException: virtual OptimizerException {};

}
}
