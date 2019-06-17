#!/usr/bin/env bash

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
# (c) 2019 polynomial contributors.
#------------------------------------------------------------------------------
source test/externalTests/common.sh

verify_input "$1"
POLJSON="$1"

function install_fn { npm install; }
function compile_fn { npx susyknot compile; }
function test_fn { npm test; }

function gnosis_safe_test
{
    OPTIMIZER_LEVEL=1
    setup https://github.com/gnosis/safe-contracts.git development
    run_install install_fn

    CONFIG=$(find_susyknot_config)
    replace_libpolc_call

    run_test compile_fn test_fn
}

external_test Gnosis-Safe gnosis_safe_test

