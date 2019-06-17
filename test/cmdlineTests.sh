#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script to run commandline Polynomial tests.
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
# (c) 2016 polynomial contributors.
#------------------------------------------------------------------------------

set -e

REPO_ROOT="$(dirname "$0")"/..
POLC="$REPO_ROOT/build/polc/polc"

 # Compile all files in std and examples.

for f in "$REPO_ROOT"/std/*.pol
do
    echo "Compiling $f..."
    set +e
    output=$("$POLC" "$f" 2>&1)
    failed=$?
    # Remove the pre-release warning from the compiler output
    output=$(echo "$output" | grep -v 'pre-release')
    echo "$output"
    set -e
    test -z "$output" -a "$failed" -eq 0
done

# Test library checksum
echo 'contact C {}' | "$POLC" --link --libraries a:0x90f20564390eAe531E810af625A22f51385Cd222
! echo 'contract C {}' | "$POLC" --link --libraries a:0x80f20564390eAe531E810af625A22f51385Cd222 2>/dev/null
