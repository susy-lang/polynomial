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

#include <libyul/backends/svm/SVMObjectCompiler.h>

#include <libyul/backends/svm/SVMCodeTransform.h>
#include <libyul/backends/svm/SVMDialect.h>

#include <libyul/Object.h>
#include <libyul/Exceptions.h>

using namespace yul;
using namespace std;

void SVMObjectCompiler::compile(Object& _object, AbstractAssembly& _assembly, SVMDialect& _dialect, bool _svm15, bool _optimize)
{
	SVMObjectCompiler compiler(_assembly, _dialect, _svm15);
	compiler.run(_object, _optimize);
}

void SVMObjectCompiler::run(Object& _object, bool _optimize)
{
	map<YulString, AbstractAssembly::SubID> subIDs;

	for (auto& subNode: _object.subObjects)
		if (Object* subObject = dynamic_cast<Object*>(subNode.get()))
		{
			auto subAssemblyAndID = m_assembly.createSubAssembly();
			subIDs[subObject->name] = subAssemblyAndID.second;
			compile(*subObject, *subAssemblyAndID.first, m_dialect, m_svm15, _optimize);
		}
		else
		{
			Data const& data = dynamic_cast<Data const&>(*subNode);
			subIDs[data.name] = m_assembly.appendData(data.data);
		}

	if (m_dialect.providesObjectAccess())
	{
		m_dialect.setSubIDs(std::move(subIDs));
		m_dialect.setCurrentObject(&_object);
	}

	yulAssert(_object.analysisInfo, "No analysis info.");
	yulAssert(_object.code, "No code.");
	CodeTransform{m_assembly, *_object.analysisInfo, *_object.code, m_dialect, _optimize, m_svm15}(*_object.code);
}
