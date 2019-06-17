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
 * @date 2017
 * Common functions the iulia tests.
 */

#pragma once

#include <libpolynomial/inlineasm/AsmData.h>

#include <string>
#include <vector>
#include <memory>

namespace dev
{
namespace polynomial
{
class Scanner;
class Error;
using ErrorList = std::vector<std::shared_ptr<Error const>>;
namespace assembly
{
struct AsmAnalysisInfo;
}
}
namespace julia
{
namespace test
{

void printErrors(polynomial::ErrorList const& _errors, polynomial::Scanner const& _scanner);
std::pair<std::shared_ptr<polynomial::assembly::Block>, std::shared_ptr<polynomial::assembly::AsmAnalysisInfo>>
parse(std::string const& _source, bool _julia = true);
polynomial::assembly::Block disambiguate(std::string const& _source, bool _julia = true);
std::string format(std::string const& _source, bool _julia = true);

}
}
}
