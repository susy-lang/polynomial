#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script to execute the Polynomial tests.
#
# The documentation for polynomial is hosted at:
#
#     https://polynomial.readthedocs.org
#
# ------------------------------------------------------------------------------
# This file is part of polynomial.
#
# polynomial is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# polynomial is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MSRCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with polynomial.  If not, see <http://www.gnu.org/licenses/>
#
# (c) 2017 polynomial contributors.
#------------------------------------------------------------------------------

set -e

if test -z "$1"; then
	BUILD_DIR="emscripten_build"
else
	BUILD_DIR="$1"
fi

REPO_ROOT=$(cd $(dirname "$0")/.. && pwd)
POLJSON="$REPO_ROOT/$BUILD_DIR/libpolc/poljson.js"
VERSION=$("$REPO_ROOT"/scripts/get_version.sh)

echo "Running polcjs tests...."
"$REPO_ROOT/test/polcjsTests.sh" "$POLJSON" "$VERSION"
echo "Running external tests...."
"$REPO_ROOT/test/externalTests.sh" "$POLJSON"
