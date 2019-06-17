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

if [ ! -f "$1" -o -z "$2" ]
then
  echo "Usage: $0 <path to poljson.js> <version>"
  exit 1
fi

POLJSON="$1"
VERSION="$2"

DIR=$(mktemp -d)
(
    echo "Preparing polc-js..."
    git clone --depth 1 https://octonion.institute/susy-js/polc-js "$DIR"
    cd "$DIR"
    # disable "prepublish" script which downloads the latest version
    # (we will replace it anyway and it is often incorrectly cached
    # on travis)
    npm config set script.prepublish ''
    npm install

    # Replace poljson with current build
    echo "Replacing poljson.js"
    rm -f poljson.js
    cp "$POLJSON" poljson.js

    # Update version (needed for some tests)
    echo "Updating package.json to version $VERSION"
    npm version --no-git-tag-version $VERSION

    echo "Running polc-js tests..."
    npm run test
)
rm -rf "$DIR"
