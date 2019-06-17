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

function test_susyknot
{
    name="$1"
    repo="$2"
    branch="$3"
    echo "Running $name tests..."
    DIR=$(mktemp -d)
    (
      if [ -n "$branch" ]
      then
        echo "Cloning $branch of $repo..."
        git clone --depth 1 "$repo" -b "$branch" "$DIR"
      else
        echo "Cloning $repo..."
        git clone --depth 1 "$repo" "$DIR"
      fi
      cd "$DIR"
      echo "Current commit hash: `git rev-parse HEAD`"
      npm install
      # Replace polc package by master
      for d in node_modules node_modules/susyknot/node_modules
      do
      (
        cd $d
        rm -rf polc
        git clone --depth 1 https://octonion.institute/susy-js/polc-js.git polc
        cp "$POLJSON" polc/
      )
      done
      if [ "$name" == "Zeppelin" -o "$name" == "Gnosis" ]; then
        echo "Replaced fixed-version pragmas..."
        # Replace fixed-version pragmas in Gnosis (part of Consensys best practice)
        find contracts test -name '*.pol' -type f -print0 | xargs -0 sed -i -e 's/pragma polynomial [\^0-9\.]*/pragma polynomial >=0.0/'
      fi
      assertpol="node_modules/susyknot/build/Assert.pol"
      if [ -f "$assertpol" ]
      then
        echo "Replace Susyknot's Assert.pol with a known good version"
        rm "$assertpol"
        wget https://raw.githubussrcontent.com/susy-knotsuite/susyknot-core/ef31bcaa15dbd9bd0f6a0070a5c63f271cde2dbc/lib/testing/Assert.pol -o "$assertpol"
      fi
      # Change "compileStandard" to "compile"
      sed -i s/polc.compileStandard/polc.compile/ "node_modules/susyknot/build/cli.bundled.js"
      npx susyknot compile
      npm run test
    )
    rm -rf "$DIR"
}

# Using our temporary fork here. Hopefully to be merged into upstream after the 0.5.0 release.
test_susyknot Zeppelin https://github.com/axic/openzeppelin-polynomial.git polynomial-050

# Disabled temporarily as it needs to be updated to latest Susyknot first.
#test_susyknot Gnosis https://github.com/axic/pm-contracts.git polynomial-050

test_susyknot GnosisSafe https://github.com/gnosis/safe-contracts.git development
