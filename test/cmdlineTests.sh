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

REPO_ROOT=$(cd $(dirname "$0")/.. && pwd)
echo $REPO_ROOT
POLC="$REPO_ROOT/build/polc/polc"

FULLARGS="--optimize --ignore-missing --combined-json abi,asm,ast,bin,bin-runtime,compact-format,devdoc,hashes,interface,metadata,opcodes,srcmap,srcmap-runtime,userdoc"

echo "Checking that the bug list is up to date..."
"$REPO_ROOT"/scripts/update_bugs_by_version.py

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

printTask "Testing unknown options..."
(
    set +e
    output=$("$POLC" --allow=test 2>&1)
    failed=$?
    set -e

    if [ "$output" == "unrecognised option '--allow=test'" ] && [ $failed -ne 0 ] ; then
	echo "Passed"
    else
	printError "Incorrect response to unknown options: $STDERR"
	exit 1
    fi
)

# General helper function for testing POLC behaviour, based on file name, compile opts, exit code, stdout and stderr.
# An failure is expected.
test_polc_file_input_failures() {
    local filename="${1}"
    local polc_args="${2}"
    local stdout_expected="${3}"
    local stderr_expected="${4}"
    local stdout_path=`mktemp`
    local stderr_path=`mktemp`

    set +e
    "$POLC" "${filename}" ${polc_args} 1>$stdout_path 2>$stderr_path
    exitCode=$?
    set -e

    sed -i -e '/^Warning: This is a pre-release compiler version, please do not use it in production./d' "$stderr_path"
    sed -i -e 's/ Consider adding "pragma .*$//' "$stderr_path"

    if [[ $exitCode -eq 0 ]]; then
        printError "Incorrect exit code. Expected failure (non-zero) but got success (0)."
        rm -f $stdout_path $stderr_path
        exit 1
    fi

    if [[ "$(cat $stdout_path)" != "${stdout_expected}" ]]; then
        printError "Incorrect output on stderr received. Expected:"
        echo -e "${stdout_expected}"

        printError "But got:"
        cat $stdout_path
        rm -f $stdout_path $stderr_path
        exit 1
    fi

    if [[ "$(cat $stderr_path)" != "${stderr_expected}" ]]; then
        printError "Incorrect output on stderr received. Expected:"
        echo -e "${stderr_expected}"

        printError "But got:"
        cat $stderr_path
        rm -f $stdout_path $stderr_path
        exit 1
    fi

    rm -f $stdout_path $stderr_path
}

printTask "Testing passing files that are not found..."
test_polc_file_input_failures "file_not_found.pol" "" "" "\"file_not_found.pol\" is not found."

printTask "Testing passing files that are not files..."
test_polc_file_input_failures "." "" "" "\".\" is not a valid file."

printTask "Testing passing empty remappings..."
test_polc_file_input_failures "${0}" "=/some/remapping/target" "" "Invalid remapping: \"=/some/remapping/target\"."
test_polc_file_input_failures "${0}" "ctx:=/some/remapping/target" "" "Invalid remapping: \"ctx:=/some/remapping/target\"."

printTask "Testing passing location printing..."
(
cd "$REPO_ROOT"/test/cmdlineErrorReports/
for file in *.pol
do
    ret=`cat $file.ref`
    test_polc_file_input_failures "$file" "" "" "$ret"
done
)

printTask "Compiling various other contracts and libraries..."
(
cd "$REPO_ROOT"/test/compilationTests/
for dir in *
do
    if [ "$dir" != "README.md" ]
    then
        echo " - $dir"
        cd "$dir"
        compileFull -w *.pol */*.pol
        cd ..
    fi
done
)

printTask "Compiling all examples from the documentation..."
POLTMPDIR=$(mktemp -d)
(
    set -e
    cd "$REPO_ROOT"
    REPO_ROOT=$(pwd) # make it absolute
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

test_polc_assembly_output() {
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

printTask "Testing assemble, yul, strict-assembly and optimize..."
(
    echo '{}' | "$POLC" - --assemble &>/dev/null
    echo '{}' | "$POLC" - --yul &>/dev/null
    echo '{}' | "$POLC" - --strict-assembly &>/dev/null

    # Test options above in conjunction with --optimize.
    # Using both, --assemble and --optimize should fail.
    ! echo '{}' | "$POLC" - --assemble --optimize &>/dev/null

    # Test yul and strict assembly output
    # Non-empty code results in non-empty binary representation with optimizations turned off,
    # while it results in empty binary representation with optimizations turned on.
    test_polc_assembly_output "{ let x:u256 := 0:u256 }" "{ let x:u256 := 0:u256 }" "--yul"
    test_polc_assembly_output "{ let x:u256 := 0:u256 }" "{ }" "--yul --optimize"

    test_polc_assembly_output "{ let x := 0 }" "{ let x := 0 }" "--strict-assembly"
    test_polc_assembly_output "{ let x := 0 }" "{ }" "--strict-assembly --optimize"
)


printTask "Testing standard input..."
POLTMPDIR=$(mktemp -d)
(
    set +e
    output=$("$POLC" --bin  2>&1)
    result=$?
    set -e

    # This should fail
    if [[ !("$output" =~ "No input files given") || ($result == 0) ]] ; then
        printError "Incorrect response to empty input arg list: $STDERR"
        exit 1
    fi

    set +e
    output=$(echo 'contract C {} ' | "$POLC" - --bin 2>/dev/null | grep -q "<stdin>:C")
    result=$?
    set -e

    # The contract should be compiled
    if [[ "$result" != 0 ]] ; then
        exit 1
    fi

    # This should not fail
    set +e
    output=$(echo '' | "$POLC" --ast - 2>/dev/null)
    set -e
    if [[ $? != 0 ]] ; then
        exit 1
    fi
)

printTask "Testing poljson via the fuzzer..."
POLTMPDIR=$(mktemp -d)
(
    set -e
    cd "$REPO_ROOT"
    REPO_ROOT=$(pwd) # make it absolute
    cd "$POLTMPDIR"
    "$REPO_ROOT"/scripts/isolate_tests.py "$REPO_ROOT"/test/
    "$REPO_ROOT"/scripts/isolate_tests.py "$REPO_ROOT"/docs/ docs
    for f in *.pol
    do
        set +e
        "$REPO_ROOT"/build/test/tools/polfuzzer --quiet < "$f"
        if [ $? -ne 0 ]; then
            printError "Fuzzer failed on:"
            cat "$f"
            exit 1
        fi

        "$REPO_ROOT"/build/test/tools/polfuzzer --without-optimizer --quiet < "$f"
        if [ $? -ne 0 ]; then
            printError "Fuzzer (without optimizer) failed on:"
            cat "$f"
            exit 1
        fi
        set -e
    done
)
rm -rf "$POLTMPDIR"
echo "Commandline tests successful."
