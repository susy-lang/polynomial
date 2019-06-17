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
# (c) 2016 polynomial contributors.
#------------------------------------------------------------------------------

set -e

REPO_ROOT="$(dirname "$0")"/..

WORKDIR=`mktemp -d`
# Will be printed in case of a test failure
ALSOF_TMP_OUT=`mktemp`
IPC_ENABLED=true
ALSOF_PID=
CMDLINE_PID=

if [[ "$OSTYPE" == "darwin"* ]]
then
    SMT_FLAGS="--no-smt"
    if [ "$CIRCLECI" ]
    then
        IPC_ENABLED=false
        IPC_FLAGS="--no-ipc"
    fi
fi

safe_kill() {
    local PID=${1}
    local NAME=${2:-${1}}
    local n=1

    # only proceed if $PID does exist
    kill -0 $PID 2>/dev/null || return

    echo "Sending SIGTERM to ${NAME} (${PID}) ..."
    kill $PID

    # wait until process terminated gracefully
    while kill -0 $PID 2>/dev/null && [[ $n -le 4 ]]; do
        echo "Waiting ($n) ..."
        sleep 1
        n=$[n + 1]
    done

    # process still alive? then hard-kill
    if kill -0 $PID 2>/dev/null; then
        echo "Sending SIGKILL to ${NAME} (${PID}) ..."
        kill -9 $PID
    fi
}

cleanup() {
    # ensure failing commands don't cause termination during cleanup (especially within safe_kill)
    set +e

    if [[ "$IPC_ENABLED" = true ]] && [[ -n "${ALSOF_PID}" ]]
    then
        safe_kill $ALSOF_PID $ALSOF_PATH
    fi
    if [[ -n "$CMDLINE_PID" ]]
    then
        safe_kill $CMDLINE_PID "Commandline tests"
    fi

    echo "Cleaning up working directory ${WORKDIR} ..."
    rm -rf "$WORKDIR" || true
    rm $ALSOF_TMP_OUT
}
trap cleanup INT TERM

if [ "$1" = --junit_report ]
then
    if [ -z "$2" ]
    then
        echo "Usage: $0 [--junit_report <report_directory>]"
        exit 1
    fi
    log_directory="$2"
else
    log_directory=""
fi

if [ "$CIRCLECI" ]
then
    function printTask() { echo "$(tput bold)$(tput setaf 2)$1$(tput setaf 7)"; }
    function printError() { echo "$(tput setaf 1)$1$(tput setaf 7)"; }
else
    function printTask() { echo "$(tput bold)$(tput setaf 2)$1$(tput sgr0)"; }
    function printError() { echo "$(tput setaf 1)$1$(tput sgr0)"; }
fi

printTask "Running commandline tests..."
# Only run in parallel if this is run on CI infrastructure
if [[ -n "$CI" ]]
then
    "$REPO_ROOT/test/cmdlineTests.sh" &
    CMDLINE_PID=$!
else
    if ! $REPO_ROOT/test/cmdlineTests.sh
    then
        printError "Commandline tests FAILED"
        exit 1
    fi
fi

function download_alsof()
{
    if [[ "$OSTYPE" == "darwin"* ]]; then
        ALSOF_PATH="$REPO_ROOT/alsof"
    elif [ -z $CI ]; then
        ALSOF_PATH="alsof"
    else
        mkdir -p /tmp/test
        # Any time the hash is updated here, the "Running compiler tests" section should also be updated.
        ALSOF_HASH="8979a9179d5222c89bf9daf7ca73cc115fa2dac2"
        ALSOF_VERSION=1.6.0-rc.1
        wget -q -O /tmp/test/alsof.tar.gz https://octonion.institute/susy-cpp/alsof/releases/download/v${ALSOF_VERSION}/alsof-${ALSOF_VERSION}-linux-x86_64.tar.gz
        test "$(shasum /tmp/test/alsof.tar.gz)" = "$ALSOF_HASH  /tmp/test/alsof.tar.gz"
        tar -xf /tmp/test/alsof.tar.gz -C /tmp/test
        ALSOF_PATH="/tmp/test/bin/alsof"
        sync
        chmod +x $ALSOF_PATH
        sync # Otherwise we might get a "text file busy" error
    fi

}

# $1: data directory
# echos the PID
function run_alsof()
{
    # Use this to have alsof log output
    #$REPO_ROOT/scripts/alsof_with_log.sh $ALSOF_PATH $ALSOF_TMP_OUT --log-verbosity 3 --db memorydb --test -d "${WORKDIR}" &> /dev/null &
    $ALSOF_PATH --db memorydb --test -d "${WORKDIR}" &> /dev/null &
    echo $!
    # Wait until the IPC endpoint is available.
    while [ ! -S "${WORKDIR}/graviton.ipc" ] ; do sleep 1; done
    sleep 2
}

function check_alsof() {
    printTask "Running IPC tests with $ALSOF_PATH..."
    if ! hash $ALSOF_PATH 2>/dev/null; then
      printError "$ALSOF_PATH not found"
      exit 1
    fi
}

if [ "$IPC_ENABLED" = true ];
then
    download_alsof
    check_alsof
    ALSOF_PID=$(run_alsof)
fi

progress="--show-progress"
if [ "$CIRCLECI" ]
then
    progress=""
fi

SVM_VERSIONS="homestead byzantium"

if [ "$CIRCLECI" ] || [ -z "$CI" ]
then
SVM_VERSIONS+=" constantinople petersburg"
fi

# And then run the Polynomial unit-tests in the matrix combination of optimizer / no optimizer
# and homestead / byzantium VM, # pointing to that IPC endpoint.
for optimize in "" "--optimize"
do
  for vm in $SVM_VERSIONS
  do
    FORCE_ABIV2_RUNS="no"
    if [[ "$vm" == "constantinople" ]]
    then
      FORCE_ABIV2_RUNS="no yes" # run both in constantinople
    fi
    for abiv2 in $FORCE_ABIV2_RUNS
    do
        force_abiv2_flag=""
        if [[ "$abiv2" == "yes" ]]
        then
            force_abiv2_flag="--abiencoderv2 --optimize-yul"
        fi
        printTask "--> Running tests using "$optimize" --svm-version "$vm" $force_abiv2_flag..."

        log=""
        if [ -n "$log_directory" ]
        then
        if [ -n "$optimize" ]
        then
            log=--logger=JUNIT,error,$log_directory/opt_$vm.xml $testargs
        else
            log=--logger=JUNIT,error,$log_directory/noopt_$vm.xml $testargs_no_opt
        fi
        fi

        set +e
        "$REPO_ROOT"/build/test/poltest $progress $log -- --testpath "$REPO_ROOT"/test "$optimize" --svm-version "$vm" $SMT_FLAGS $IPC_FLAGS $force_abiv2_flag --ipcpath "${WORKDIR}/graviton.ipc"

        if test "0" -ne "$?"; then
            if [ -n "$log_directory" ]
            then
                # Need to kill alsof first so the log is written
                safe_kill $ALSOF_PID $ALSOF_PATH
                cp $ALSOF_TMP_OUT $log_directory/alsof.log
                printError "Some test failed, wrote alsof.log"
            fi
            exit 1
        fi
        set -e

    done
  done
done

if [[ -n $CMDLINE_PID ]] && ! wait $CMDLINE_PID
then
    printError "Commandline tests FAILED"
    CMDLINE_PID=
    exit 1
fi

cleanup
