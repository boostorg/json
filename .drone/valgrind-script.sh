#!/bin/bash

set -ex

echo '==================================> BEFORE_INSTALL'

echo '==================================> INSTALL'

git clone https://github.com/boostorg/boost-ci.git boost-ci-cloned --depth 1
cp -prf boost-ci-cloned/ci .
rm -rf boost-ci-cloned
# source ci/travis/install.sh
# The contents of install.sh below:

export SELF=`basename $DRONE_REPO`
export BOOST_CI_TARGET_BRANCH="$DRONE_COMMIT_BRANCH"
export BOOST_CI_SRC_FOLDER=$(pwd)
. ./ci/common_install.sh

echo '==================================> COMPILE'

cd $BOOST_ROOT/libs/$SELF
ci/travis/valgrind.sh
