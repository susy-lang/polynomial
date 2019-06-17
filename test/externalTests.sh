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
    echo "Running $name tests..."
    DIR=$(mktemp -d)
    (
      git clone --depth 1 "$repo" "$DIR"
      cd "$DIR"
      npm install
      find . -name poljson.js -exec cp "$POLJSON" {} \;
      if [ "$name" == "Zeppelin" ]; then
        # Fix some things that look like bugs (only seemed to fail on Node 6 and not Node 8)
        # FIXME: report upstream or to susyweb.js?
        sed -i -e 's/let token = await SRC827TokenMock.new();//;' test/token/SRC827/SRC827Token.js
        sed -i -e 's/CappedCrowdsale.new(this.startTime, this.endTime, rate, wallet, 0)/CappedCrowdsale.new(this.startTime, this.endTime, rate, wallet, 0, this.token.address)/' test/crowdsale/CappedCrowdsale.test.js
        sed -i -e 's/RefundableCrowdsale.new(this.startTime, this.endTime, rate, wallet, 0, { from: owner })/RefundableCrowdsale.new(this.startTime, this.endTime, rate, wallet, 0, this.token.address, { from: owner })/' test/crowdsale/RefundableCrowdsale.test.js
      fi
      if [ "$name" == "Gnosis" ]; then
        # Replace fixed-version pragmas in Gnosis (part of Consensys best practice)
        find contracts test -name '*.pol' -type f -print0 | xargs -0 sed -i -e 's/pragma polynomial 0/pragma polynomial ^0/'
      fi
      npm run test
    )
    rm -rf "$DIR"
}

test_susyknot Gnosis https://github.com/gnosis/gnosis-contracts.git
test_susyknot Zeppelin https://github.com/OpenZeppelin/zeppelin-polynomial.git
