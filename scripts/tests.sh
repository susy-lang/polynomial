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

echo "Running commandline tests..."
"$REPO_ROOT/test/cmdlineTests.sh"

echo "Checking that StandardToken.pol, owned.pol and mortal.pol produce bytecode..."
output=$("$REPO_ROOT"/build/polc/polc --bin "$REPO_ROOT"/std/*.pol 2>/dev/null | grep "ffff" | wc -l)
test "${output//[[:blank:]]/}" = "3"

# This conditional is only needed because we don't have a working Homebrew
# install for `sof` at the time of writing, so we unzip the ZIP file locally
# instead.  This will go away soon.
if [[ "$OSTYPE" == "darwin"* ]]; then
    SOF_PATH="$REPO_ROOT/sof"
else
    SOF_PATH="sof"
fi

# This trailing ampersand directs the shell to run the command in the background,
# that is, it is forked and run in a separate sub-shell, as a job,
# asynchronously. The shell will immediately return the return status of 0 for
# true and continue as normal, either processing further commands in a script
# or returning the cursor focus back to the user in a Linux terminal.
$SOF_PATH --test -d /tmp/test &
SOF_PID=$!

# Wait until the IPC endpoint is available.  That won't be available instantly.
# The node needs to get a little way into its startup sequence before the IPC
# is available and is ready for the unit-tests to start talking to it.
while [ ! -S /tmp/test/graviton.ipc ]; do sleep 2; done
echo "--> IPC available."
sleep 2
# And then run the Polynomial unit-tests (once without optimization, once with),
# pointing to that IPC endpoint.
echo "--> Running tests without optimizer..."
  "$REPO_ROOT"/build/test/poltest --show-progress -- --ipcpath /tmp/test/graviton.ipc && \
  echo "--> Running tests WITH optimizer..." && \
  "$REPO_ROOT"/build/test/poltest --show-progress -- --optimize --ipcpath /tmp/test/graviton.ipc
ERROR_CODE=$?
pkill "$SOF_PID" || true
sleep 4
pgrep "$SOF_PID" && pkill -9 "$SOF_PID" || true
exit $ERROR_CODE
