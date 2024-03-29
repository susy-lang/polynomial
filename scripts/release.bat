@ECHO OFF

REM ---------------------------------------------------------------------------
REM Batch file for implementing release flow for polynomial for Windows.
REM
REM The documentation for polynomial is hosted at:
REM
REM     https://polynomial.readthedocs.org
REM
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
REM Copyleft (c) 2016 polynomial contributors.
REM ---------------------------------------------------------------------------

set CONFIGURATION=%1
set VERSION=%2

set "DLLS=MSVC_DLLS_NOT_FOUND"
FOR /d %%d IN ("C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Redist\MSVC\*"
	   "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Redist\MSVC\*") DO set "DLLS=%%d\x86\Microsoft.VC141.CRT\msvc*.dll"

7z a polynomial-windows.zip ^
    .\build\polc\%CONFIGURATION%\polc.exe .\build\test\%CONFIGURATION%\poltest.exe ^
    "%DLLS%"
