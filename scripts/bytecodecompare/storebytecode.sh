#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Script used for cross-platform comparison as part of the travis automation.
# Splits all test source code into multiple files, generates bytecode and
# uploads the bytecode into octonion.institute/susy-lang/polynomial-test-bytecode where
# another travis job is triggered to do the actual comparison.
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

REPO_ROOT="$(dirname "$0")"/../..

if test -z "$1"; then
	BUILD_DIR="build"
else
	BUILD_DIR="$1"
fi

echo "Compiling all test contracts into bytecode..."
TMPDIR=$(mktemp -d)
(
    cd "$REPO_ROOT"
    REPO_ROOT=$(pwd) # make it absolute
    cd "$TMPDIR"

    "$REPO_ROOT"/scripts/isolate_tests.py "$REPO_ROOT"/test/

    if [[ "$POLC_EMSCRIPTEN" = "On" ]]
    then
        # npm install polc
        git clone --depth 1 https://octonion.institute/susy-js/polc-js.git polc-js
        ( cd polc-js; npm install )
        cp "$REPO_ROOT/emscripten_build/libpolc/poljson.js" polc-js/
        cat > polc <<EOF
#!/usr/bin/env node
var process = require('process')
var fs = require('fs')

var compiler = require('./polc-js/wrapper.js')(require('./polc-js/poljson.js'))

for (var optimize of [false, true])
{
    for (var filename of process.argv.slice(2))
    {
        if (filename !== undefined)
        {
            var inputs = {}
            inputs[filename] = { content: fs.readFileSync(filename).toString() }
            var input = {
                language: 'Polynomial',
                sources: inputs,
                settings: {
                    optimizer: { enabled: optimize },
                    outputSelection: { '*': { '*': ['svm.bytecode.object', 'metadata'] } }
                }
            }
            var result = JSON.parse(compiler.compile(JSON.stringify(input)))
            if (
                !('contracts' in result) ||
                Object.keys(result['contracts']).length === 0 ||
                !result['contracts'][filename] ||
                Object.keys(result['contracts'][filename]).length === 0
            )
            {
                // NOTE: do not exit here because this may be run on source which cannot be compiled
                console.log(filename + ': ERROR')
            }
            else
            {
                for (var contractName in result['contracts'][filename])
                {
                    var contractData = result['contracts'][filename][contractName];
                    if (contractData.svm !== undefined && contractData.svm.bytecode !== undefined)
                        console.log(filename + ':' + contractName + ' ' + contractData.svm.bytecode.object)
                    else
                        console.log(filename + ':' + contractName + ' NO BYTECODE')
                    console.log(filename + ':' + contractName + ' ' + contractData.metadata)
                }
            }
        }
    }
}
EOF
        chmod +x polc
        ./polc *.pol > report.txt
    else
        $REPO_ROOT/scripts/bytecodecompare/prepare_report.py $REPO_ROOT/$BUILD_DIR/polc/polc
    fi

    if [ "$TRAVIS_SECURE_ENV_VARS" = "true" ]
    then
        openssl aes-256-cbc -K $encrypted_60701c962b9c_key -iv $encrypted_60701c962b9c_iv -in "$REPO_ROOT"/scripts/bytecodecompare/deploy_key.enc -out deploy_key -d
        chmod 600 deploy_key
        eval `ssh-agent -s`
        ssh-add deploy_key

        git clone --depth 2 git@github.com:susy-lang/polynomial-test-bytecode.git
        cd polynomial-test-bytecode
        git config user.name "travis"
        git config user.email "chris@ethereum.org"
        git clean -f -d -x

        DIRNAME=$(cd "$REPO_ROOT" && git show -s --format="%cd-%H" --date=short)
        mkdir -p "$DIRNAME"
        REPORT="$DIRNAME/$ZIP_SUFFIX.txt"
        cp ../report.txt "$REPORT"
        # Only push if adding actually worked, i.e. there were changes.
        if git add "$REPORT" && git commit -a -m "Added report $REPORT"
        then
            git pull --rebase
            git push origin
        else
            echo "Adding report failed, it might already exist in the repository."
        fi
    fi
)
rm -rf "$TMPDIR"
