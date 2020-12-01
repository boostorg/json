#!/bin/bash

set -ex
export TRAVIS_BUILD_DIR=$(pwd)
export TRAVIS_BRANCH=$DRONE_BRANCH
export VCS_COMMIT_ID=$DRONE_COMMIT
export GIT_COMMIT=$DRONE_COMMIT

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
echo "Before common_install.sh"
echo "SELF IS $SELF and BOOST_ROOT is $BOOST_ROOT X"
pwd

. ./ci/common_install.sh

echo "After common_install.sh"
echo "SELF IS $SELF and BOOST_ROOT is $BOOST_ROOT X"
pwd

echo '==================================> COMPILE'

cd $BOOST_ROOT/libs/$SELF
ci/travis/codecov.sh
