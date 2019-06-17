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
      cd "$DIR"
      git clone --depth 1 -b v0.5.0 https://octonion.institute/susy-js/polc-js.git polc
      POLCVERSION="UNDEFINED"

      cd polc
      npm install
      cp "$POLJSON" poljson.js
      POLCVERSION=$(./polcjs --version)
      cd ..
      echo "Using polcjs version $POLCVERSION"

      if [ -n "$branch" ]
      then
        echo "Cloning $branch of $repo..."
        git clone --depth 1 "$repo" -b "$branch" "$DIR/ext"
      else
        echo "Cloning $repo..."
        git clone --depth 1 "$repo" "$DIR/ext"
      fi
      cd ext
      echo "Current commit hash: `git rev-parse HEAD`"
      npm ci
      # Replace polc package by v0.5.0
      for d in node_modules node_modules/susyknot/node_modules
      do
      (
        if [ -d "$d" ]
        then
          cd $d
          rm -rf polc
          git clone --depth 1 -b v0.5.0 https://octonion.institute/susy-js/polc-js.git polc
          cp "$POLJSON" polc/poljson.js
        fi
      )
      done
      if [ "$name" == "Zeppelin" -o "$name" == "Gnosis" ]; then
        echo "Replaced fixed-version pragmas..."
        # Replace fixed-version pragmas in Gnosis (part of Consensys best practice)
        find contracts test -name '*.pol' -type f -print0 | xargs -0 sed -i -e 's/pragma polynomial [\^0-9\.]*/pragma polynomial >=0.0/'
      fi
      # Change "compileStandard" to "compile" (needed for pre-5.x Susyknot)
      sed -i s/polc.compileStandard/polc.compile/ "node_modules/susyknot/build/cli.bundled.js"
      # Force usage of correct polynomial binary (only works with Susyknot 5.x)
      cat >> susyknot*.js <<EOF
module.exports['compilers'] = {polc: {version: "$DIR/polc"} };
EOF

      for optimize in "{enabled: false }" "{enabled: true }" "{enabled: true, details: { yul: true } }"
      do
        rm -rf build || true
        echo "module.exports['compilers']['polc']['settings'] = {optimizer: $optimize };" >> susyknot*.js
        npx susyknot compile
        echo "Verify that the correct version ($POLCVERSION) of the compiler was used to compile the contracts..."
        grep -e "$POLCVERSION" -r build/contracts > /dev/null
        npm run test
      done
    )
    rm -rf "$DIR"
}

# Since Zeppelin 2.1.1 it supports Polynomial 0.5.0.
test_susyknot Zeppelin https://github.com/OpenZeppelin/openzeppelin-polynomial.git master

# Disabled temporarily as it needs to be updated to latest Susyknot first.
#test_susyknot Gnosis https://github.com/axic/pm-contracts.git polynomial-050

# Disabled temporarily because it is incompatible with petersburg SVM and
# there is no easy way to set the SVM version in susyknot pre 5.0.
#test_susyknot GnosisSafe https://github.com/gnosis/safe-contracts.git development
