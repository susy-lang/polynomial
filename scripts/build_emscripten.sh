#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script for building Polynomial for emscripten.
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

if [[ "$OSTYPE" != "darwin"* ]]; then
    ./scripts/travis-emscripten/install_deps.sh
    docker run -v $(pwd):/root/project -w /root/project trzeci/emscripten:sdk-tag-1.37.21-64bit ./scripts/travis-emscripten/build_emscripten.sh
fi
