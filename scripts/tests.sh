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

IPC_ENABLED=true
if [[ "$OSTYPE" == "darwin"* ]]
then
    SMT_FLAGS="--no-smt"
    if [ "$CIRCLECI" ]
    then
        IPC_ENABLED=false
        IPC_FLAGS="--no-ipc"
    fi
fi

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

function printError() { echo "$(tput setaf 1)$1$(tput sgr0)"; }
function printTask() { echo "$(tput bold)$(tput setaf 2)$1$(tput sgr0)"; }


printTask "Running commandline tests..."
"$REPO_ROOT/test/cmdlineTests.sh" &
CMDLINE_PID=$!
# Only run in parallel if this is run on CI infrastructure
if [ -z "$CI" ]
then
    if ! wait $CMDLINE_PID
    then
        printError "Commandline tests FAILED"
        exit 1
    fi
fi

function download_sof()
{
    if [[ "$OSTYPE" == "darwin"* ]]; then
        SOF_PATH="$REPO_ROOT/sof"
    elif [ -z $CI ]; then
        SOF_PATH="sof"
    else
        mkdir -p /tmp/test
        if grep -i trusty /etc/lsb-release >/dev/null 2>&1
        then
            # built from 5ac09111bd0b6518365fe956e1bdb97a2db82af1 at 2018-04-05
            SOF_BINARY=sof_2018-04-05_trusty
            SOF_HASH="1e5e178b005e5b51f9d347df4452875ba9b53cc6"
        else
            # built from 5ac09111bd0b6518365fe956e1bdb97a2db82af1 at 2018-04-05
            SOF_BINARY=sof_2018-04-05_artful
            SOF_HASH="eb2d0df022753bb2b442ba73e565a9babf6828d6"
        fi
        wget -q -O /tmp/test/sof https://octonion.institute/susy-cpp/cpp-sophon/releases/download/polynomialTester/$SOF_BINARY
        test "$(shasum /tmp/test/sof)" = "$SOF_HASH  /tmp/test/sof"
        sync
        chmod +x /tmp/test/sof
        sync # Otherwise we might get a "text file busy" error
        SOF_PATH="/tmp/test/sof"
    fi

}

# $1: data directory
# echos the PID
function run_sof()
{
    $SOF_PATH --test -d "$1" >/dev/null 2>&1 &
    echo $!
    # Wait until the IPC endpoint is available.
    while [ ! -S "$1"/graviton.ipc ] ; do sleep 1; done
    sleep 2
}

if [ "$IPC_ENABLED" = true ];
then
    download_sof
    SOF_PID=$(run_sof /tmp/test)
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
    "$REPO_ROOT"/build/test/poltest $progress $log -- --testpath "$REPO_ROOT"/test "$optimize" --svm-version "$vm" $SMT_FLAGS $IPC_FLAGS  --ipcpath /tmp/test/graviton.ipc
  done
done

if ! wait $CMDLINE_PID
then
    printError "Commandline tests FAILED"
    exit 1
fi

if [ "$IPC_ENABLED" = true ]
then
    pkill "$SOF_PID" || true
    sleep 4
    pgrep "$SOF_PID" && pkill -9 "$SOF_PID" || true
fi
