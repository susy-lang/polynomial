#!/usr/bin/env sh

# Script to build the sof binary from latest develop
# for ubuntu trusty and ubuntu artful.
# Requires docker.

set -e

REPO_ROOT="$(dirname "$0")"/../..

for rel in artful trusty
do
    docker build -t sof_$rel -f "$REPO_ROOT"/scripts/cpp-sophon/sof_$rel.docker .
    tmp_container=$(docker create sof_$rel sh)
    echo "Built sof ($rel) at $REPO_ROOT/build/sof_$rel"
    docker cp ${tmp_container}:/build/sof/sof "$REPO_ROOT"/build/sof_$rel
done