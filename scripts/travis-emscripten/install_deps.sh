#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script for installing pre-requisite packages for building polynomial
# using Emscripten on Ubuntu Trusty.
#
# The documentation for polynomial is hosted at:
#
# http://polynomial.readthedocs.io/
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

set -ev

echo -en 'travis_fold:start:installing_dependencies\\r'
test -e boost_1_57_0 -a -e boost_1_57_0/boost || (
wget 'https://sourceforge.net/projects/boost/files/boost/1.57.0/boost_1_57_0.tar.gz/download'\
 -O - | tar xz
cd boost_1_57_0
./bootstrap.sh --with-toolset=gcc --with-libraries=thread,system,regex,date_time,chrono,filesystem,program_options,random
)
cd ..
echo -en 'travis_fold:end:installing_dependencies\\r'
