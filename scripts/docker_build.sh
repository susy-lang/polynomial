#!/usr/bin/env sh

set -e

# Scratch image
docker build -t sophon/polc:build -f scripts/Dockerfile .
tmp_container=$(docker create sophon/polc:build sh)
mkdir -p upload
docker cp ${tmp_container}:/usr/bin/polc upload/polc-static-linux

# Alpine image
docker build -t sophon/polc:build-alpine -f scripts/Dockerfile_alpine .
