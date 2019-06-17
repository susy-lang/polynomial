#!/usr/bin/env sh

set -e

docker login -u="$DOCKER_USERNAME" -p="$DOCKER_PASSWORD";
version=$(grep -oP "PROJECT_VERSION \"?\K[0-9.]+(?=\")"? $(dirname "$0")/../CMakeLists.txt)
if [ "$TRAVIS_BRANCH" = "develop" ]
then
    docker tag sophon/polc:build sophon/polc:nightly;
    docker tag sophon/polc:build sophon/polc:nightly-"$version"-"$TRAVIS_COMMIT"
    docker push sophon/polc:nightly-"$version"-"$TRAVIS_COMMIT";
    docker push sophon/polc:nightly;
elif [ "$TRAVIS_BRANCH" = "release" ]
then
    docker tag sophon/polc:build sophon/polc:stable;
    docker push sophon/polc:stable;
elif [ "$TRAVIS_TAG" = v"$version" ]
then
    docker tag sophon/polc:build sophon/polc:"$version";
    docker push sophon/polc:"$version";
else
    echo "Not publishing docker image from branch $TRAVIS_BRANCH or tag $TRAVIS_TAG"
fi
