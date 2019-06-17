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

# There is an implicit assumption here that we HAVE to run from root directory.
REPO_ROOT=$(pwd)

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

# Wait until the IPC endpoint is available.  That won't be available instantly.
# The node needs to get a little way into its startup sequence before the IPC
# is available and is ready for the unit-tests to start talking to it.
while [ ! -S /tmp/test/graviton.ipc ]; do sleep 2; done

# And then run the Polynomial unit-tests, pointing to that IPC endpoint.
"$REPO_ROOT"/build/test/poltest --ipc /tmp/test/graviton.ipc
ERROR_CODE=$?
pkill sof || true
sleep 4
pgrep sof && pkill -9 sof || true
exit $ERROR_CODE
