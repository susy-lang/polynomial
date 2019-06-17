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
        # Any time the hash is updated here, the "Running compiler tests" section should also be updated.
        mkdir -p /tmp/test
        if grep -i trusty /etc/lsb-release >/dev/null 2>&1
        then
            # built from d661ac4fec0aeffbedcdc195f67f5ded0c798278 at 2018-06-20
            ALSOF_BINARY=alsof_2018-06-20_trusty
            ALSOF_HASH="54b8a5455e45b295e3a962f353ff8f1580ed106c"
        else
            # built from d661ac4fec0aeffbedcdc195f67f5ded0c798278 at 2018-06-20
            ALSOF_BINARY=alsof_2018-06-20_artful
            ALSOF_HASH="02e6c4b3d98299885e73f7db6c9e3fbe3d66d444"
        fi
        ALSOF_PATH="/tmp/test/alsof"
        wget -q -O $ALSOF_PATH https://octonion.institute/susy-cpp/cpp-sophon/releases/download/polynomialTester/$ALSOF_BINARY
        test "$(shasum $ALSOF_PATH)" = "$ALSOF_HASH  $ALSOF_PATH"
        sync
        chmod +x $ALSOF_PATH
        sync # Otherwise we might get a "text file busy" error
    fi

}

# $1: data directory
# echos the PID
function run_alsof()
{
    $ALSOF_PATH --test -d "${WORKDIR}" >/dev/null 2>&1 &
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
SVM_VERSIONS+=" constantinople"
fi

# And then run the Polynomial unit-tests in the matrix combination of optimizer / no optimizer
# and homestead / byzantium VM, # pointing to that IPC endpoint.
for optimize in "" "--optimize"
do
  for vm in $SVM_VERSIONS
  do
    printTask "--> Running tests using "$optimize" --svm-version "$vm"..."
    log=""
    if [ -n "$log_directory" ]
    then
      if [ -n "$optimize" ]
      then
        log=--logger=JUNIT,test_suite,$log_directory/opt_$vm.xml $testargs
      else
        log=--logger=JUNIT,test_suite,$log_directory/noopt_$vm.xml $testargs_no_opt
      fi
    fi
    "$REPO_ROOT"/build/test/poltest $progress $log -- --testpath "$REPO_ROOT"/test "$optimize" --svm-version "$vm" $SMT_FLAGS $IPC_FLAGS  --ipcpath "${WORKDIR}/graviton.ipc"
  done
done

if [[ -n $CMDLINE_PID ]] && ! wait $CMDLINE_PID
then
    printError "Commandline tests FAILED"
    CMDLINE_PID=
    exit 1
fi

cleanup
