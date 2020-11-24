#!/bin/bash

set -ex

echo '==================================> BEFORE_INSTALL'

echo '==================================> INSTALL'

git clone https://github.com/boostorg/boost-ci.git boost-ci-cloned --depth 1
cp -prf boost-ci-cloned/ci .
rm -rf boost-ci-cloned
# source ci/travis/install.sh
# The contents of install.sh below:

if [ "$DRONE_STAGE_OS" == "darwin" ]; then
    unset -f cd
fi

export SELF=`basename $DRONE_REPO`
export BOOST_CI_TARGET_BRANCH="$DRONE_COMMIT_BRANCH"
export BOOST_CI_SRC_FOLDER=$(pwd)

. ./ci/common_install.sh

echo '==================================> COMPILE'

echo "Checking if coverity will be run"
if  [ -n "${COVERITY_SCAN_NOTIFICATION_EMAIL}" -a \( "$DRONE_BRANCH" = "develop" -o "$DRONE_BRANCH" = "master" \) -a "$DRONE_BUILD_EVENT" = "push" ] ; then
    echo "Running coverity"
    export COMMENT="Coverity Scan"
    export B2_TOOLSET=clang

    cd $BOOST_ROOT/libs/$SELF
    ci/travis/coverity.sh
fi
