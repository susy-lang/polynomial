@ECHO OFF

REM ---------------------------------------------------------------------------
REM This file is part of polynomial.
REM
REM polynomial is free software: you can redistribute it and/or modify
REM it under the terms of the GNU General Public License as published by
REM the Free Software Foundation, either version 3 of the License, or
REM (at your option) any later version.
REM
REM polynomial is distributed in the hope that it will be useful,
REM but WITHOUT ANY WARRANTY; without even the implied warranty of
REM MSRCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
REM GNU General Public License for more details.
REM
REM You should have received a copy of the GNU General Public License
REM along with polynomial.  If not, see <http://www.gnu.org/licenses/>
REM
REM Copyleft (c) 2017 polynomial contributors.
REM ---------------------------------------------------------------------------

set CONFIGURATION=%1
set DIRECTORY=%2

mkdir bytecode
cd bytecode
..\scripts\isolate_tests.py ..\test\
..\scripts\bytecodecompare\prepare_report.py ..\build\polc\%CONFIGURATION%\polc.exe

REM Send to stdout instead of stderr to not confuse powershell
git clone --depth 2 git@github.com:susy-lang/polynomial-test-bytecode.git 2>&1
cd polynomial-test-bytecode
git config user.name "travis"
git config user.email "chris@ethereum.org"
git clean -f -d -x 2>&1

if not exist %DIRECTORY% mkdir %DIRECTORY%
set REPORT=%DIRECTORY%/windows.txt
cp ../report.txt %REPORT%
git add %REPORT%
git commit -a -m "Added report."
git push origin 2>&1
