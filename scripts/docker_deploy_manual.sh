#!/usr/bin/env sh

set -e

if [ -z "$1" ]
then
    echo "Usage: $0 <tag/branch>"
    exit 1
fi
branch="$1"

#docker login

DIR=$(mktemp -d)
(
cd "$DIR"

git clone --depth 2 https://octonion.institute/susy-lang/polynomial.git -b "$branch"
cd polynomial
commithash=$(git rev-parse --short=8 HEAD)
echo -n "$commithash" > commit_hash.txt
version=$($(dirname "$0")/get_version.sh)
if [ "$branch" = "release" -o "$branch" = v"$version" ]
then
    echo -n > prerelease.txt
else
    date -u +"nightly.%Y.%-m.%-d" > prerelease.txt
fi

rm -rf .git
docker build -t sophon/polc:build -f scripts/Dockerfile .
tmp_container=$(docker create sophon/polc:build sh)
if [ "$branch" = "develop" ]
then
    docker tag sophon/polc:build sophon/polc:nightly;
    docker tag sophon/polc:build sophon/polc:nightly-"$version"-"$commithash"
    docker push sophon/polc:nightly-"$version"-"$commithash";
    docker push sophon/polc:nightly;
elif [ "$branch" = v"$version" ]
then
    docker tag sophon/polc:build sophon/polc:stable;
    docker tag sophon/polc:build sophon/polc:"$version";
    docker push sophon/polc:stable;
    docker push sophon/polc:"$version";
else
    echo "Not publishing docker image from branch or tag $branch"
fi
)
rm -rf "$DIR"
