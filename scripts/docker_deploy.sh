#!/usr/bin/env sh

set -e

docker login -u="$DOCKER_USERNAME" -p="$DOCKER_PASSWORD";
version=$($(dirname "$0")/get_version.sh)
if [ "$TRAVIS_BRANCH" = "develop" ]
then
    docker tag sophon/polc:build sophon/polc:nightly;
    docker tag sophon/polc:build sophon/polc:nightly-"$version"-"$TRAVIS_COMMIT"
    docker push sophon/polc:nightly-"$version"-"$TRAVIS_COMMIT";
    docker push sophon/polc:nightly;
elif [ "$TRAVIS_TAG" = v"$version" ]
then
    docker tag sophon/polc:build sophon/polc:stable;
    docker tag sophon/polc:build sophon/polc:"$version";
    docker push sophon/polc:stable;
    docker push sophon/polc:"$version";
else
    echo "Not publishing docker image from branch $TRAVIS_BRANCH or tag $TRAVIS_TAG"
fi
