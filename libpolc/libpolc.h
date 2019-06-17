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
 * @author Christian <c@sofdev.com>
 * @date 2014
 * Public compiler API.
 */

#pragma once

#include <stdbool.h>

#ifdef __cplusplus
#define POLC_NOEXCEPT noexcept
#else
#define POLC_NOEXCEPT
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// Callback used to retrieve additional source files.
///
/// "Returns" two pointers that should be heap-allocated and are free'd by the caller.
typedef void (*CStyleReadFileCallback)(char const* _path, char** o_contents, char** o_error);

/// Returns the complete license document.
///
/// The pointer returned must not be freed by the caller.
char const* polynomial_license() POLC_NOEXCEPT;

/// Returns the compiler version.
///
/// The pointer returned must not be freed by the caller.
char const* polynomial_version() POLC_NOEXCEPT;

/// Takes a "Standard Input JSON" and an optional callback (can be set to null). Returns
/// a "Standard Output JSON". Both are to be UTF-8 encoded.
///
/// The pointer returned must not be freed by the caller.
char const* polynomial_compile(char const* _input, CStyleReadFileCallback _readCallback) POLC_NOEXCEPT;

/// Frees up any allocated memory.
///
/// NOTE: the pointer returned by polynomial_compile is invalid after calling this!
void polynomial_free() POLC_NOEXCEPT;

#ifdef __cplusplus
}
#endif
