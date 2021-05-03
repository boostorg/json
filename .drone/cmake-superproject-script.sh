#!/bin/bash

set -ex

echo '==================================> BEFORE_INSTALL'

if  [ "$DRONE_BEFORE_INSTALL" = "gcc11" ]; then
    wget http://kayari.org/gcc-latest/gcc-latest.deb
    sudo dpkg -i gcc-latest.deb
    export PATH=/opt/gcc-latest/bin:$PATH
    export LD_RUN_PATH=/opt/gcc-latest/lib64
fi

echo '==================================> INSTALL'

git clone https://github.com/boostorg/boost-ci.git boost-ci-cloned --depth 1
cp -prf boost-ci-cloned/ci .
rm -rf boost-ci-cloned
# source ci/travis/install.sh
# The contents of install.sh below:

if [ "$DRONE_STAGE_OS" = "darwin" ]; then
    unset -f cd
fi

export SELF=`basename $DRONE_REPO`
export BOOST_CI_TARGET_BRANCH="$DRONE_COMMIT_BRANCH"
export BOOST_CI_SRC_FOLDER=$(pwd)

. ./ci/common_install.sh

echo '==================================> COMPILE'

export CXXFLAGS="-Wall -Wextra -Werror"

mkdir __build_static
cd __build_static
cmake -DBOOST_ENABLE_CMAKE=1 -DBUILD_TESTING=ON -DBoost_VERBOSE=1 \
    -DBOOST_INCLUDE_LIBRARIES=json ..
cmake --build .
ctest --output-on-failure -R boost_json

cd ..

mkdir __build_shared
cd __build_shared
cmake -DBOOST_ENABLE_CMAKE=1 -DBUILD_TESTING=ON -DBoost_VERBOSE=1 \
    -DBOOST_INCLUDE_LIBRARIES=json -DBUILD_SHARED_LIBS=ON ..
cmake --build .
ctest --output-on-failure -R boost_json
