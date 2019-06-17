#!/usr/bin/env bash

#------------------------------------------------------------------------------
# Bash script for publishing Polynomial Emscripten binaries to Github.
#
# The results are committed to https://octonion.institute/susy-lang/polc-bin.
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

set -e

VER=$(cat CMakeLists.txt | grep 'set(PROJECT_VERSION' | sed -e 's/.*set(PROJECT_VERSION "\(.*\)".*/\1/')
test -n "$VER"
VER="v$VER"
COMMIT=$(git rev-parse --short HEAD)
DATE=$(date --date="$(git log -1 --date=iso --format=%ad HEAD)" --utc +%F)

ENCRYPTED_KEY_VAR="encrypted_${ENCRYPTION_LABEL}_key"
ENCRYPTED_IV_VAR="encrypted_${ENCRYPTION_LABEL}_iv"
ENCRYPTED_KEY=${!ENCRYPTED_KEY_VAR}
ENCRYPTED_IV=${!ENCRYPTED_IV_VAR}
openssl aes-256-cbc -K $ENCRYPTED_KEY -iv $ENCRYPTED_IV -in scripts/travis-emscripten/deploy_key.enc -out deploy_key -d
chmod 600 deploy_key
eval `ssh-agent -s`
ssh-add deploy_key

git clone --depth 2 git@github.com:sophon/polc-bin.git
cd polc-bin
git config user.name "travis"
git config user.email "chris@ethereum.org"
git checkout -B gh-pages origin/gh-pages
git clean -f -d -x
# We only want one release per day and we do not want to push the same commit twice.
if ls ./bin/poljson-"$VER-$DATE"-*.js ./bin/poljson-*-"$COMMIT.js" > /dev/null
then
  true
else
  # This file is assumed to be the product of the build_emscripten.sh script.
  cp ../poljson.js ./bin/"poljson-$VER-$DATE-$COMMIT.js"
  ./update-index.sh
  cd bin
  LATEST=$(ls -r poljson-v* | head -n 1)
  cp "$LATEST" poljson-latest.js
  cp poljson-latest.js ../poljson.js
  git add .
  git add ../poljson.js
  git commit -m "Added compiler version $LATEST"
  git push origin gh-pages
fi
