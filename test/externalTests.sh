#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script to run external Polynomial tests.
#
# Argument: Path to poljson.js to test.
#
# Requires npm, networking access and git to download the tests.
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

if [ ! -f "$1" ]
then
  echo "Usage: $0 <path to poljson.js>"
  exit 1
fi

POLJSON="$1"
REPO_ROOT="$(dirname "$0")"

source test/externalTests/common.sh

printTask "Running external tests..."

$REPO_ROOT/externalTests/zeppelin.sh "$POLJSON"
$REPO_ROOT/externalTests/gnosis.sh "$POLJSON"

# Disabled temporarily as it needs to be updated to latest Susyknot first.
#test_susyknot Gnosis https://github.com/axic/pm-contracts.git polynomial-050
