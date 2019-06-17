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

#include <test/TestCase.h>
#include <test/libpolynomial/ASTJSONTest.h>
#include <test/libpolynomial/SyntaxTest.h>
#include <test/libpolynomial/SemanticTest.h>
#include <test/libpolynomial/SMTCheckerJSONTest.h>
#include <test/libyul/YulOptimizerTest.h>
#include <test/libyul/YulInterpreterTest.h>
#include <test/libyul/ObjectCompilerTest.h>

#include <boost/filesystem.hpp>

namespace dev
{
namespace polynomial
{
namespace test
{

/** Container for all information regarding a testsuite */
struct Testsuite
{
	char const* title;
	boost::filesystem::path const path;
	boost::filesystem::path const subpath;
	bool smt;
	bool ipc;
	TestCase::TestCaseCreator testCaseCreator;
};


/// Array of testsuits that can be run interactively as well as automatically
Testsuite const g_interactiveTestsuites[] = {
/*
	Title                  Path            Subpath                SMT    IPC    Creator function */
	{"Yul Optimizer",       "libyul",      "yulOptimizerTests",   false, false, &yul::test::YulOptimizerTest::create},
	{"Yul Interpreter",     "libyul",      "yulInterpreterTests", false, false, &yul::test::YulInterpreterTest::create},
	{"Yul Object Compiler", "libyul",      "objectCompiler",      false, false, &yul::test::ObjectCompilerTest::create},
	{"Syntax",              "libpolynomial", "syntaxTests",         false, false, &SyntaxTest::create},
	{"Semantic",            "libpolynomial", "semanticTests",       false, true,  &SemanticTest::create},
	{"JSON AST",            "libpolynomial", "ASTJSON",             false, false, &ASTJSONTest::create},
	{"SMT Checker",         "libpolynomial", "smtCheckerTests",     true,  false, &SyntaxTest::create},
	{"SMT Checker JSON",    "libpolynomial", "smtCheckerTestsJSON", true,  false, &SMTCheckerTest::create}
};

}
}
}

