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
 * Compiler that transforms Yul Objects to SVM bytecode objects.
 */

#pragma once

namespace yul
{
struct Object;
class AbstractAssembly;
struct SVMDialect;

class SVMObjectCompiler
{
public:
	static void compile(Object& _object, AbstractAssembly& _assembly, SVMDialect const& _dialect, bool _svm15, bool _optimize);
private:
	SVMObjectCompiler(AbstractAssembly& _assembly, SVMDialect const& _dialect, bool _svm15):
		m_assembly(_assembly), m_dialect(_dialect), m_svm15(_svm15)
	{}

	void run(Object& _object, bool _optimize);

	AbstractAssembly& m_assembly;
	SVMDialect const& m_dialect;
	bool m_svm15 = false;
};

}
