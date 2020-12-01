#!/bin/bash

set -ex
export TRAVIS_BUILD_DIR=$(pwd)
export TRAVIS_BRANCH=$DRONE_BRANCH

echo '==================================> BEFORE_INSTALL'

pip install --user https://github.com/codecov/codecov-python/archive/master.zip

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
echo "Before common_install.sh"
pwd

shopt -s dotglob
. ./ci/common_install.sh
shopt -u dotglob

echo "After common_install.sh"
pwd

echo '==================================> COMPILE'

cd $BOOST_ROOT/libs/$SELF

# ci/travis/codecov.sh
# The contents of ci/travis/codecov.sh are shown below.

#. $(dirname "${BASH_SOURCE[0]}")/../enforce.sh
. ./ci/enforce.sh

if [ -z "$GCOV" ]; then
    ver=7 # default
    if [ "${B2_TOOLSET%%-*}" == "gcc" ]; then
        if [[ "$B2_TOOLSET" =~ gcc- ]]; then
            ver="${B2_TOOLSET##*gcc-}"
        elif [[ "$TRAVIS_COMPILER" =~ gcc- ]]; then
            ver="${TRAVIS_COMPILER##*gcc-}"
        fi
    fi
    GCOV=gcov-${ver}
fi

# lcov after 1.14 needs this
# sudo apt install --no-install-recommends -y libperlio-gzip-perl libjson-perl

# install the latest lcov we know works
# some older .travis.yml files install the tip which may be unstable
rm -rf /tmp/lcov
pushd /tmp
git clone -b v1.14 https://github.com/linux-test-project/lcov.git
export PATH=/tmp/lcov/bin:$PATH
command -v lcov
lcov --version
popd

B2_VARIANT=debug
#$(dirname "${BASH_SOURCE[0]}")/build.sh cxxflags=--coverage linkflags=--coverage
./ci/build.sh cxxflags=--coverage linkflags=--coverage

#cxxflags=-fprofile-arcs cxxflags=-ftest-coverage linkflags=-fprofile-arcs linkflags=-ftest-coverage

# switch back to the original source code directory
cd $TRAVIS_BUILD_DIR

# coverage files are in ../../b2 from this location
lcov --gcov-tool=$GCOV --rc lcov_branch_coverage=${LCOV_BRANCH_COVERAGE:-1} --base-directory "$BOOST_ROOT/libs/$SELF" --directory "$BOOST_ROOT" --capture --output-file all.info

# all.info contains all the coverage info for all projects - limit to ours
# first we extract the interesting headers for our project then we use that list to extract the right things
for f in `for f in include/boost/*; do echo $f; done | cut -f2- -d/`; do echo "*/$f*"; done > /tmp/interesting
echo headers that matter:
cat /tmp/interesting
xargs -L 999999 -a /tmp/interesting lcov --gcov-tool=$GCOV --rc lcov_branch_coverage=${LCOV_BRANCH_COVERAGE:-1} --extract all.info {} "*/libs/$SELF/src/*" --output-file coverage.info

# dump a summary on the console - helps us identify problems in pathing
lcov --gcov-tool=$GCOV --rc lcov_branch_coverage=${LCOV_BRANCH_COVERAGE:-1} --list coverage.info

#
# upload to codecov.io
#

#curl -s https://codecov.io/bash > .codecov
#chmod +x .codecov
#./.codecov -f $TRAVIS_BUILD_DIR/coverage.info -X gcov -x "$GCOV"
~/.local/bin/codecov -f $TRAVIS_BUILD_DIR/coverage.info -X gcov
