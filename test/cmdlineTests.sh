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

## GLOBAL VARIABLES

REPO_ROOT=$(cd $(dirname "$0")/.. && pwd)
POLC="$REPO_ROOT/build/polc/polc"
INTERACTIVE=true
if ! tty -s || [ "$CI" ]
then
    INTERACTIVE=""
fi

FULLARGS="--optimize --ignore-missing --combined-json abi,asm,ast,bin,bin-runtime,compact-format,devdoc,hashes,interface,metadata,opcodes,srcmap,srcmap-runtime,userdoc"

## FUNCTIONS

if [ "$CIRCLECI" ]
then
    function printTask() { echo "$(tput bold)$(tput setaf 2)$1$(tput setaf 7)"; }
    function printError() { echo "$(tput setaf 1)$1$(tput setaf 7)"; }
else
    function printTask() { echo "$(tput bold)$(tput setaf 2)$1$(tput sgr0)"; }
    function printError() { echo "$(tput setaf 1)$1$(tput sgr0)"; }
fi


function compileFull()
{
    local expected_exit_code=0
    local expect_output=0
    if [[ $1 = '-e' ]]
    then
        expected_exit_code=1
        expect_output=1
        shift;
    fi
    if [[ $1 = '-w' ]]
    then
        expect_output=1
        shift;
    fi

    local files="$*"
    local output

    local stderr_path=$(mktemp)

    set +e
    "$POLC" $FULLARGS $files >/dev/null 2>"$stderr_path"
    local exit_code=$?
    local errors=$(grep -v -E 'Warning: This is a pre-release compiler version|Warning: Experimental features are turned on|pragma experimental ABIEncoderV2|\^-------------------------------\^' < "$stderr_path")
    set -e
    rm "$stderr_path"

    if [[ \
        "$exit_code" -ne "$expected_exit_code" || \
            ( $expect_output -eq 0 && -n "$errors" ) || \
            ( $expect_output -ne 0 && -z "$errors" ) \
    ]]
    then
        printError "Unexpected compilation result:"
        printError "Expected failure: $expected_exit_code - Expected warning / error output: $expect_output"
        printError "Was failure: $exit_code"
        echo "$errors"
        printError "While calling:"
        echo "\"$POLC\" $FULLARGS $files"
        printError "Inside directory:"
        pwd
        false
    fi
}

function ask_expectation_update()
{
    if [ $INTERACTIVE ]
    then
        local newExpectation="${1}"
        local expectationFile="${2}"
        while true;
        do
            read -p "(u)pdate expectation/(q)uit? "
            case $REPLY in
                u* ) echo "$newExpectation" > $expectationFile ; break;;
                q* ) exit 1;;
            esac
        done
    fi
}

# General helper function for testing POLC behaviour, based on file name, compile opts, exit code, stdout and stderr.
# An failure is expected.
function test_polc_behaviour()
{
    local filename="${1}"
    local polc_args="${2}"
    local polc_stdin="${3}"
    [ -z "$polc_stdin"  ] && polc_stdin="/dev/stdin"
    local stdout_expected="${4}"
    local exit_code_expected="${5}"
    local stderr_expected="${6}"
    local stdout_expectation_file="${7}" # the file to write to when user chooses to update stdout expectation
    local stderr_expectation_file="${8}" # the file to write to when user chooses to update stderr expectation
    local stdout_path=`mktemp`
    local stderr_path=`mktemp`

    trap "rm -f $stdout_path $stderr_path" EXIT

    if [[ "$exit_code_expected" = "" ]]; then exit_code_expected="0"; fi

    local polc_command="$POLC ${filename} ${polc_args} <$polc_stdin"
    set +e
    "$POLC" "${filename}" ${polc_args} <"$polc_stdin" >"$stdout_path" 2>"$stderr_path"
    exitCode=$?
    set -e

    if [[ "$polc_args" == *"--standard-json"* ]]
    then
        sed -i -e 's/{[^{]*Warning: This is a pre-release compiler version[^}]*},\{0,1\}//' "$stdout_path"
        sed -i -e 's/"errors":\[\],\{0,1\}//' "$stdout_path"
    else
        sed -i -e '/^Warning: This is a pre-release compiler version, please do not use it in production./d' "$stderr_path"
        sed -i -e 's/ Consider adding "pragma .*$//' "$stderr_path"
    fi
    # Remove path to cpp file
    sed -i -e 's/^\(Exception while assembling:\).*/\1/' "$stderr_path"
    # Remove exception class name.
    sed -i -e 's/^\(Dynamic exception type:\).*/\1/' "$stderr_path"

    if [[ $exitCode -ne "$exit_code_expected" ]]
    then
        printError "Incorrect exit code. Expected $exit_code_expected but got $exitCode."
        exit 1
    fi

    if [[ "$(cat $stdout_path)" != "${stdout_expected}" ]]
    then
        printError "Incorrect output on stdout received. Expected:"
        echo -e "${stdout_expected}"

        printError "But got:"
        echo -e "$(cat $stdout_path)"

        printError "When running $polc_command"

        if [ -n "$stdout_expectation_file" ]
        then
            ask_expectation_update "$(cat $stdout_path)" "$stdout_expectation_file"
        else
            exit 1
        fi
    fi

    if [[ "$(cat $stderr_path)" != "${stderr_expected}" ]]
    then
        printError "Incorrect output on stderr received. Expected:"
        echo -e "${stderr_expected}"

        printError "But got:"
        echo -e "$(cat $stderr_path)"

        printError "When running $polc_command"

        if [ -n "$stderr_expectation_file" ]
        then
            ask_expectation_update "$(cat $stderr_path)" "$stderr_expectation_file"
        else
            exit 1
        fi
    fi
}


function test_polc_assembly_output()
{
    local input="${1}"
    local expected="${2}"
    local polc_args="${3}"

    local expected_object="object \"object\" { code "${expected}" }"

    output=$(echo "${input}" | "$POLC" - ${polc_args} 2>/dev/null)
    empty=$(echo $output | sed -ne '/'"${expected_object}"'/p')
    if [ -z "$empty" ]
    then
        printError "Incorrect assembly output. Expected: "
        echo -e ${expected}
        printError "with arguments ${polc_args}, but got:"
        echo "${output}"
        exit 1
    fi
}

## RUN

echo "Checking that the bug list is up to date..."
"$REPO_ROOT"/scripts/update_bugs_by_version.py

printTask "Testing unknown options..."
(
    set +e
    output=$("$POLC" --allow=test 2>&1)
    failed=$?
    set -e

    if [ "$output" == "unrecognised option '--allow=test'" ] && [ $failed -ne 0 ]
    then
        echo "Passed"
    else
        printError "Incorrect response to unknown options: $STDERR"
        exit 1
    fi
)


printTask "Testing passing files that are not found..."
test_polc_behaviour "file_not_found.pol" "" "" "" 1 "\"file_not_found.pol\" is not found." "" ""

printTask "Testing passing files that are not files..."
test_polc_behaviour "." "" "" "" 1 "\".\" is not a valid file." "" ""

printTask "Testing passing empty remappings..."
test_polc_behaviour "${0}" "=/some/remapping/target" "" "" 1 "Invalid remapping: \"=/some/remapping/target\"." "" ""
test_polc_behaviour "${0}" "ctx:=/some/remapping/target" "" "" 1 "Invalid remapping: \"ctx:=/some/remapping/target\"." "" ""

printTask "Running general commandline tests..."
(
    cd "$REPO_ROOT"/test/cmdlineTests/
    for tdir in */
    do
        if [ -e "${tdir}/input.json" ]
        then
            inputFile=""
            stdin="${tdir}/input.json"
            stdout="$(cat ${tdir}/output.json 2>/dev/null || true)"
            stdoutExpectationFile="${tdir}/output.json"
            args="--standard-json "$(cat ${tdir}/args 2>/dev/null || true)
        else
            inputFile="${tdir}input.pol"
            stdin=""
            stdout="$(cat ${tdir}/output 2>/dev/null || true)"
            stdoutExpectationFile="${tdir}/output"
            args=$(cat ${tdir}/args 2>/dev/null || true)
        fi
        exitCode=$(cat ${tdir}/exit 2>/dev/null || true)
        err="$(cat ${tdir}/err 2>/dev/null || true)"
        stderrExpectationFile="${tdir}/err"
        printTask " - ${tdir}"
        test_polc_behaviour "$inputFile" \
                            "$args" \
                            "$stdin" \
                            "$stdout" \
                            "$exitCode" \
                            "$err" \
                            "$stdoutExpectationFile" \
                            "$stderrExpectationFile"
    done
)

printTask "Compiling various other contracts and libraries..."
(
    cd "$REPO_ROOT"/test/compilationTests/
    for dir in */
    do
        echo " - $dir"
        cd "$dir"
        compileFull -w *.pol */*.pol
        cd ..
    done
)

printTask "Compiling all examples from the documentation..."
POLTMPDIR=$(mktemp -d)
(
    set -e
    cd "$POLTMPDIR"
    "$REPO_ROOT"/scripts/isolate_tests.py "$REPO_ROOT"/docs/ docs
    for f in *.pol
    do
        # The contributors guide uses syntax tests, but we cannot
        # really handle them here.
        if grep -E 'DeclarationError:|// ----' "$f" >/dev/null
        then
            continue
        fi
        echo "$f"
        opts=''
        # We expect errors if explicitly stated, or if imports
        # are used (in the style guide)
        if grep -E "This will not compile|import \"" "$f" >/dev/null
        then
            opts="-e"
        fi
        if grep "This will report a warning" "$f" >/dev/null
        then
            opts="$opts -w"
        fi
        compileFull $opts "$POLTMPDIR/$f"
    done
)
rm -rf "$POLTMPDIR"
echo "Done."

printTask "Testing library checksum..."
echo '' | "$POLC" - --link --libraries a:0x90f20564390eAe531E810af625A22f51385Cd222 >/dev/null
! echo '' | "$POLC" - --link --libraries a:0x80f20564390eAe531E810af625A22f51385Cd222 &>/dev/null

printTask "Testing long library names..."
echo '' | "$POLC" - --link --libraries aveeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeerylonglibraryname:0x90f20564390eAe531E810af625A22f51385Cd222 >/dev/null

printTask "Testing linking itself..."
POLTMPDIR=$(mktemp -d)
(
    cd "$POLTMPDIR"
    set -e
    echo 'library L { function f() public pure {} } contract C { function f() public pure { L.f(); } }' > x.pol
    "$POLC" --bin -o . x.pol 2>/dev/null
    # Explanation and placeholder should be there
    grep -q '//' C.bin && grep -q '__' C.bin
    # But not in library file.
    grep -q -v '[/_]' L.bin
    # Now link
    "$POLC" --link --libraries x.pol:L:0x90f20564390eAe531E810af625A22f51385Cd222 C.bin
    # Now the placeholder and explanation should be gone.
    grep -q -v '[/_]' C.bin
)
rm -rf "$POLTMPDIR"

printTask "Testing overwriting files..."
POLTMPDIR=$(mktemp -d)
(
    set -e
    # First time it works
    echo 'contract C {} ' | "$POLC" - --bin -o "$POLTMPDIR/non-existing-stuff-to-create" 2>/dev/null
    # Second time it fails
    ! echo 'contract C {} ' | "$POLC" - --bin -o "$POLTMPDIR/non-existing-stuff-to-create" 2>/dev/null
    # Unless we force
    echo 'contract C {} ' | "$POLC" - --overwrite --bin -o "$POLTMPDIR/non-existing-stuff-to-create" 2>/dev/null
)
rm -rf "$POLTMPDIR"

printTask "Testing assemble, yul, strict-assembly and optimize..."
(
    echo '{}' | "$POLC" - --assemble &>/dev/null
    echo '{}' | "$POLC" - --yul &>/dev/null
    echo '{}' | "$POLC" - --strict-assembly &>/dev/null

    # Test options above in conjunction with --optimize.
    # Using both, --assemble and --optimize should fail.
    ! echo '{}' | "$POLC" - --assemble --optimize &>/dev/null
    ! echo '{}' | "$POLC" - --yul --optimize &>/dev/null

    # Test yul and strict assembly output
    # Non-empty code results in non-empty binary representation with optimizations turned off,
    # while it results in empty binary representation with optimizations turned on.
    test_polc_assembly_output "{ let x:u256 := 0:u256 }" "{ let x:u256 := 0:u256 }" "--yul"
    test_polc_assembly_output "{ let x := 0 }" "{ let x := 0 }" "--strict-assembly"
    test_polc_assembly_output "{ let x := 0 }" "{ { } }" "--strict-assembly --optimize"
)


printTask "Testing standard input..."
POLTMPDIR=$(mktemp -d)
(
    set +e
    output=$("$POLC" --bin  2>&1)
    result=$?
    set -e

    # This should fail
    if [[ !("$output" =~ "No input files given") || ($result == 0) ]]
    then
        printError "Incorrect response to empty input arg list: $STDERR"
        exit 1
    fi

    set +e
    output=$(echo 'contract C {} ' | "$POLC" - --bin 2>/dev/null | grep -q "<stdin>:C")
    result=$?
    set -e

    # The contract should be compiled
    if [[ "$result" != 0 ]]
    then
        exit 1
    fi

    # This should not fail
    set +e
    output=$(echo '' | "$POLC" --ast - 2>/dev/null)
    set -e
    if [[ $? != 0 ]]
    then
        exit 1
    fi
)

printTask "Testing poljson via the fuzzer..."
POLTMPDIR=$(mktemp -d)
(
    set -e
    cd "$POLTMPDIR"
    "$REPO_ROOT"/scripts/isolate_tests.py "$REPO_ROOT"/test/
    "$REPO_ROOT"/scripts/isolate_tests.py "$REPO_ROOT"/docs/ docs

    echo *.pol | xargs -P 4 -n 50 "$REPO_ROOT"/build/test/tools/polfuzzer --quiet --input-files
    echo *.pol | xargs -P 4 -n 50 "$REPO_ROOT"/build/test/tools/polfuzzer --without-optimizer --quiet --input-files
)
rm -rf "$POLTMPDIR"

echo "Commandline tests successful."
