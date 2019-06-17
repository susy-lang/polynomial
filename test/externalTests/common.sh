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
set -e

if [ "$CIRCLECI" ]
then
    function printTask() { echo ""; echo "$(tput bold)$(tput setaf 2)$1$(tput setaf 7)"; }
    function printError() { echo ""; echo "$(tput setaf 1)$1$(tput setaf 7)"; }
    function printLog() { echo "$(tput setaf 3)$1$(tput setaf 7)"; }
else
    function printTask() { echo ""; echo "$(tput bold)$(tput setaf 2)$1$(tput sgr0)"; }
    function printError() { echo ""; echo "$(tput setaf 1)$1$(tput sgr0)"; }
    function printLog() { echo "$(tput setaf 3)$1$(tput sgr0)"; }
fi

function verify_input
{
    if [ ! -f "$1" ]; then
        printError "Usage: $0 <path to poljson.js>"
        exit 1
    fi
}

function setup_polcjs
{
    local dir="$1"
    local poljson="$2"

    cd "$dir"
    printLog "Setting up polc-js..."
    git clone --depth 1 -b v0.5.0 https://octonion.institute/susy-js/polc-js.git polc

    cd polc
    npm install
    cp "$poljson" poljson.js
    POLCVERSION=$(./polcjs --version)
    printLog "Using polcjs version $POLCVERSION"
    cd ..
}

function download_project
{
    local repo="$1"
    local branch="$2"
    local dir="$3"

    printLog "Cloning $branch of $repo..."
    git clone --depth 1 "$repo" -b "$branch" "$dir/ext"
    cd ext
    echo "Current commit hash: `git rev-parse HEAD`"
}

function setup
{
    local repo="$1"
    local branch="$2"

    setup_polcjs "$DIR" "$POLJSON"
    download_project "$repo" "$branch" "$DIR"

    replace_version_pragmas
}

function replace_version_pragmas
{
    # Replace fixed-version pragmas in Gnosis (part of Consensys best practice)
    printLog "Replacing fixed-version pragmas..."
    find contracts test -name '*.pol' -type f -print0 | xargs -0 sed -i -e 's/pragma polynomial [\^0-9\.]*/pragma polynomial >=0.0/'
}

function replace_libpolc_call
{
    # Change "compileStandard" to "compile" (needed for pre-5.x Susyknot)
    printLog "Replacing libpolc compile call in Susyknot..."
    sed -i s/polc.compileStandard/polc.compile/ "node_modules/susyknot/build/cli.bundled.js"
}

function find_susyknot_config
{
    local config_file="susyknot.js"
    local alt_config_file="susyknot-config.js"

    if [ ! -f "$config_file" ] && [ ! -f "$alt_config_file" ]; then
        printError "No matching Susyknot config found."
    fi
    if [ ! -f "$config_file" ]; then
        config_file=alt_config_file
    fi
    echo "$config_file"
}

function force_polc_susyknot_modules
{
    # Replace polc package by v0.5.0 and then overwrite with current version.
    printLog "Forcing polc version for all Susyknot modules..."
    for d in node_modules node_modules/susyknot/node_modules
    do
    (
        if [ -d "$d" ]; then
            cd $d
            rm -rf polc
            git clone --depth 1 -b v0.5.0 https://octonion.institute/susy-js/polc-js.git polc
            cp "$1" polc/poljson.js
        fi
    )
    done
}

function force_polc
{
    local config_file="$1"
    local dir="$2"
    local poljson="$3"

    force_polc_susyknot_modules "$poljson"

    printLog "Forcing polc version..."
    cat >> "$config_file" <<EOF
module.exports['compilers'] = {polc: {version: "$dir/polc"} };
EOF
}

function force_polc_settings
{
    local config_file="$1"
    local settings="$2"
    local svmVersion="$3"

    printLog "Forcing polc settings..."
    echo "-------------------------------------"
    echo "Config file: $config_file"
    echo "Optimizer settings: $settings"
    echo "SVM version: $svmVersion"
    echo "-------------------------------------"

    # Forcing the settings should always work by just overwriting the polc object. Forcing them by using a
    # dedicated settings objects should only be the fallback.
    echo "module.exports['polc'] = { optimizer: $settings, svmVersion: \"$svmVersion\" };" >> "$config_file"
    echo "module.exports['compilers']['polc']['settings'] = { optimizer: $settings, svmVersion: \"$svmVersion\" };" >> "$config_file"
}

function force_abi_v2
{
    # Add "pragma experimental ABIEncoderV2" to all files.
    printLog "Forcibly enabling ABIEncoderV2..."
    find contracts test -name '*.pol' -type f -print0 | \
    while IFS= read -r -d '' file
    do
        # Only add the pragma if it is not already there.
        if grep -q -v 'pragma experimental ABIEncoderV2' "$file"; then
            sed -i -e '1 i pragma experimental ABIEncoderV2;' "$file"
        fi
    done
}

function verify_compiler_version
{
    local polc_version="$1"

    printLog "Verify that the correct version ($polc_version) of the compiler was used to compile the contracts..."
    grep -e "$polc_version" -r build/contracts > /dev/null
}

function clean
{
    rm -rf build || true
}

function run_install
{
    local init_fn="$1"
    printLog "Running install function..."
    $init_fn
}

function run_test
{
    local compile_fn="$1"
    local test_fn="$2"

    force_polc "$CONFIG" "$DIR" "$POLJSON"

    printLog "Checking optimizer level..."
    if [ -z "$OPTIMIZER_LEVEL" ]; then
        printError "Optimizer level not found. Please define OPTIMIZER_LEVEL=[1, 2, 3]"
        exit 1
    fi
    if [[ "$OPTIMIZER_LEVEL" == 1 ]]; then
        declare -a optimizer_settings=("{ enabled: false }" "{ enabled: true }" "{ enabled: true, details: { yul: true } }")
    fi
    if [[ "$OPTIMIZER_LEVEL" == 2 ]]; then
        declare -a optimizer_settings=("{ enabled: true }" "{ enabled: true, details: { yul: true } }")
    fi
    if [[ "$OPTIMIZER_LEVEL" == 3 ]]; then
        declare -a optimizer_settings=("{ enabled: true, details: { yul: true } }")
    fi

    for optimize in "${optimizer_settings[@]}"
    do
        clean
        force_polc_settings "$CONFIG" "$optimize" "petersburg"
        # Force ABIEncoderV2 in the last step. Has to be the last because code is modified.
        if [ "$FORCE_ABIv2" = true ]; then
            [[ "$optimize" =~ yul ]] && force_abi_v2
        fi

        printLog "Running compile function..."
        $compile_fn
        verify_compiler_version "$POLCVERSION"
        printLog "Running test function..."
        $test_fn
    done
}

function external_test
{
    local name="$1"
    local main_fn="$2"

    printTask "Testing $name..."
    echo "==========================="
    DIR=$(mktemp -d)
    (
        if [ -z "$main_fn" ]; then
            printError "Test main function not defined."
            exit 1
        fi
        $main_fn
    )
    rm -rf "$DIR"
    echo "Done."
}

