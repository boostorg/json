#!/bin/bash

set -ex
export TRAVIS_BUILD_DIR=$(pwd)
export TRAVIS_BRANCH=$DRONE_BRANCH

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
echo "SELF IS $SELF and BOOST_ROOT is $BOOST_ROOT X"

shopt -s dotglob
. ./ci/common_install.sh
shopt -u dotglob

echo '==================================> COMPILE'

cd $BOOST_ROOT/libs/$SELF
ci/travis/codecov.sh 
