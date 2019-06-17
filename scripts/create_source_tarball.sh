#!/usr/bin/env sh
#

set -e

REPO_ROOT="$(dirname "$0")"/..
(
    cd "$REPO_ROOT"
    version=$(scripts/get_version.sh)
    commithash=$(git rev-parse --short=8 HEAD)
    commitdate=$(git show --format=%ci HEAD | head -n 1 | cut - -b1-10 | sed -e 's/-0?/./' | sed -e 's/-0?/./')

    # file exists and has zero size -> not a prerelease
    if [ -e prerelease.txt -a ! -s prerelease.txt ]
    then
        versionstring="$version"
    else
        versionstring="$version-nightly-$commitdate-$commithash"
    fi

    TEMPDIR=$(mktemp -d)
    POLDIR="$TEMPDIR/polynomial_$versionstring/"
    mkdir "$POLDIR"
    # Store the current source
    git checkout-index -a --prefix="$POLDIR"
    # Store the commit hash
    echo "$commithash" > "$POLDIR/commit_hash.txt"
    if [ -e prerelease.txt -a ! -s prerelease.txt ]
    then
        cp prerelease.txt "$POLDIR/"
    fi
    # Add dependencies
    mkdir -p "$POLDIR/deps/downloads/" 2>/dev/null || true
    wget -O "$POLDIR/deps/downloads/jsoncpp-1.8.4.tar.gz" https://github.com/open-source-parsers/jsoncpp/archive/1.8.4.tar.gz
    mkdir -p "$REPO_ROOT/upload"
    tar czf "$REPO_ROOT/upload/polynomial_$versionstring.tar.gz" -C "$TEMPDIR" "polynomial_$versionstring"
    rm -r "$TEMPDIR"
)
