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
/** @file CodeFragment.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <liblll/Exceptions.h>
#include <libsvmasm/Instruction.h>
#include <libsvmasm/Assembly.h>
#include <libdevcore/Common.h>

namespace boost { namespace spirit { class utree; } }
namespace sp = boost::spirit;

namespace dev
{
namespace lll
{

struct CompilerState;

class CodeFragment
{
public:
	using ReadCallback = std::function<std::string(std::string const&)>;

	CodeFragment() = default;
	CodeFragment(sp::utree const& _t, CompilerState& _s, ReadCallback const& _readFile, bool _allowASM = false);

	static CodeFragment compile(std::string _src, CompilerState& _s, ReadCallback const& _readFile);

	/// Consolidates data and compiles code.
	sof::Assembly& assembly(CompilerState const& _cs) { finalise(_cs); return m_asm; }

private:
	void finalise(CompilerState const& _cs);

	template <class T> static void error() { BOOST_THROW_EXCEPTION(T() ); }
	template <class T> static void error(std::string const& reason) {
		auto err = T();
		err << errinfo_comment(reason);
		BOOST_THROW_EXCEPTION(err);
	}
	void constructOperation(sp::utree const& _t, CompilerState& _s);

	bool m_finalised = false;
	sof::Assembly m_asm;
	ReadCallback m_readFile;
};

static CodeFragment const NullCodeFragment;

}
}
