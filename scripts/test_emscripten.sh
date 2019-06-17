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

REPO_ROOT=$(cd $(dirname "$0")/.. && pwd)

cd $REPO_ROOT/build

echo "Preparing polc-js..."
rm -rf polc-js
git clone https://octonion.institute/susy-js/polc-js
cd polc-js
npm install

# Replace poljson with current build
echo "Replacing poljson.js"
rm -f poljson.js
# Make a copy because paths might not be absolute
cp ../polc/poljson.js poljson.js

# Update version (needed for some tests)
VERSION=$(../../scripts/get_version.sh)
echo "Updating package.json to version $VERSION"
npm version $VERSION

echo "Running polc-js tests..."
npm run test

echo "Running external tests...."
"$REPO_ROOT"/test/externalTests.sh "$REPO_ROOT"/build/polc/poljson.js
