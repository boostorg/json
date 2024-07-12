#!/bin/bash

# Copyright 2020 Rene Rivera, Sam Darwin
# Copyright 2021 Dmitry Arkhipov (grisumbras@gmail.com)
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

set -e
export TRAVIS_OS_NAME=linux
export TRAVIS_BUILD_DIR=$(pwd)
export DRONE_BUILD_DIR=$(pwd)
export DRONE_BRANCH=${DRONE_BRANCH:-$(echo $GITHUB_REF | cut -d/ -f3-)}
export TRAVIS_BRANCH=$DRONE_BRANCH
export TRAVIS_EVENT_TYPE=$DRONE_BUILD_EVENT
export VCS_COMMIT_ID=$DRONE_COMMIT
export GIT_COMMIT=$DRONE_COMMIT
export REPO_NAME=$DRONE_REPO
export USER=$(whoami)
export CC=${CC:-gcc}
export PATH=~/.local/bin:/usr/local/bin:$PATH

common_install () {
  git clone https://github.com/boostorg/boost-ci.git boost-ci-cloned --depth 1
  cp -prf boost-ci-cloned/ci .
  rm -rf boost-ci-cloned

  if [ "$TRAVIS_OS_NAME" == "osx" ]; then
      unset -f cd
  fi

  export SELF=`basename $REPO_NAME`
  export BOOST_CI_TARGET_BRANCH="$TRAVIS_BRANCH"
  export BOOST_CI_SRC_FOLDER=$(pwd)
  : ${B2_DONT_BOOTSTRAP:=$B2_SEPARATE_BOOTSTRAP}

  . ./ci/common_install.sh

  if [ "x$B2_TOOLSET" = "xgcc-4.9" ]; then
      # this is ridiculously hacky, but the alternative is building gcc-4.9
      # for Ubuntu 18.04 manually
      pushd /etc/apt

      # temporally pretend we are Ubuntu 16.04
      sed 's/bionic/xenial/g' < sources.list > sources.list-xenial
      mv sources.list sources.list-bionic
      ln -sT sources.list-xenial sources.list
      sudo -E apt-get -o Acquire::Retries=3 update
      sudo -E DEBIAN_FRONTEND=noninteractive apt-get -o Acquire::Retries=3 -y \
          --no-install-suggests --no-install-recommends install g++-4.9

      # put everything back
      rm sources.list sources.list-xenial
      mv sources.list-bionic sources.list
      sudo -E apt-get -o Acquire::Retries=3 update

      popd
  fi

  if [ "$B2_SEPARATE_BOOTSTRAP" = 1 ]; then
    pushd tools/build
    B2_TOOLSET= ./bootstrap.sh
    popd
    cp tools/build/b2 .
  fi
}

common_cmake () {
    export CXXFLAGS="-Wall -Wextra -Werror"
    export CMAKE_SHARED_LIBS=${CMAKE_SHARED_LIBS:-1}
    export CMAKE_NO_TESTS=${CMAKE_NO_TESTS:-error}
    if [ $CMAKE_NO_TESTS = "error" ]; then
        CMAKE_BUILD_TESTING="-DBUILD_TESTING=ON"
    fi
}


if [ "$DRONE_JOB_BUILDTYPE" == "boost" ]; then

if [[ $(uname) == "Linux" && ( "$COMMENT" == tsan || "$COMMENT" == asan ) ]]; then
    echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
    sudo sysctl vm.mmap_rnd_bits=28
fi

echo '==================================> INSTALL'

common_install

echo '==================================> SCRIPT'

export B2_TARGETS=${B2_TARGETS:-"libs/$SELF/test libs/$SELF/example"}
$BOOST_ROOT/libs/$SELF/ci/travis/build.sh

elif [ "$DRONE_JOB_BUILDTYPE" == "docs" ]; then

echo '==================================> INSTALL'

export SELF=`basename $REPO_NAME`

pwd
cd ..
mkdir -p $HOME/cache && cd $HOME/cache
if [ ! -d doxygen ]; then git clone -b 'Release_1_8_15' --depth 1 https://github.com/doxygen/doxygen.git && echo "not-cached" ; else echo "cached" ; fi
cd doxygen
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release
cd build
sudo make install
cd ../..
cd ..
BOOST_BRANCH=develop && [ "$TRAVIS_BRANCH" == "master" ] && BOOST_BRANCH=master || true
git clone -b $BOOST_BRANCH https://github.com/boostorg/boost.git boost-root --depth 1
cd boost-root
export BOOST_ROOT=$(pwd)
git submodule update --init libs/context
git submodule update --init tools/boostbook
git submodule update --init tools/boostdep
git submodule update --init tools/docca
git submodule update --init tools/quickbook
rsync -av $TRAVIS_BUILD_DIR/ libs/$SELF
python tools/boostdep/depinst/depinst.py ../tools/quickbook
./bootstrap.sh
./b2 headers

echo '==================================> SCRIPT'

echo "using doxygen ; using boostbook ; using python : : python3 ;" > tools/build/src/user-config.jam
./b2 -j3 libs/$SELF/doc//boostrelease

elif [ "$DRONE_JOB_BUILDTYPE" == "codecov" ]; then

echo '==================================> INSTALL'

common_install

echo '==================================> SCRIPT'

cd $BOOST_ROOT/libs/$SELF
ci/travis/codecov.sh

elif [ "$DRONE_JOB_BUILDTYPE" == "valgrind" ]; then

echo '==================================> INSTALL'

common_install

echo '==================================> SCRIPT'

cd $BOOST_ROOT/libs/$SELF
ci/travis/valgrind.sh

elif [ "$DRONE_JOB_BUILDTYPE" == "coverity" ]; then

echo '==================================> INSTALL'

common_install

echo '==================================> SCRIPT'

if  [ -n "${COVERITY_SCAN_NOTIFICATION_EMAIL}" -a \( "$TRAVIS_BRANCH" = "develop" -o "$TRAVIS_BRANCH" = "master" \) -a \( "$DRONE_BUILD_EVENT" = "push" -o "$DRONE_BUILD_EVENT" = "cron" \) ] ; then
cd $BOOST_ROOT/libs/$SELF
ci/travis/coverity.sh
fi

elif [ "$DRONE_JOB_BUILDTYPE" == "cmake-superproject" ]; then

echo '==================================> INSTALL'

common_install

echo '==================================> COMPILE'

common_cmake

mkdir __build_static
cd __build_static
cmake -DBoost_VERBOSE=1 ${CMAKE_BUILD_TESTING} -DCMAKE_INSTALL_PREFIX=iprefix \
    -DBOOST_INCLUDE_LIBRARIES=$SELF ${CMAKE_OPTIONS} ..
if [ -n "${CMAKE_BUILD_TESTING}" ]; then
    cmake --build . --target tests
fi
cmake --build . --target install
ctest --output-on-failure --no-tests=$CMAKE_NO_TESTS -R boost_$SELF
cd ..

if [ "$CMAKE_SHARED_LIBS" = 1 ]; then

mkdir __build_shared
cd __build_shared
cmake -DBoost_VERBOSE=1 ${CMAKE_BUILD_TESTING} -DCMAKE_INSTALL_PREFIX=iprefix \
    -DBOOST_INCLUDE_LIBRARIES=$SELF -DBUILD_SHARED_LIBS=ON ${CMAKE_OPTIONS} ..
if [ -n "${CMAKE_BUILD_TESTING}" ]; then
    cmake --build . --target tests
fi
cmake --build . --target install
ctest --output-on-failure --no-tests=$CMAKE_NO_TESTS -R boost_$SELF

fi

elif [ "$DRONE_JOB_BUILDTYPE" == "cmake-mainproject" ]; then

echo '==================================> INSTALL'

common_install

echo '==================================> COMPILE'

common_cmake

mkdir __build_static
cd __build_static
cmake -DBoost_VERBOSE=1 ${CMAKE_BUILD_TESTING} -DCMAKE_INSTALL_PREFIX=iprefix \
    ${CMAKE_OPTIONS} ../libs/json
cmake --build . --target install
ctest --output-on-failure --no-tests=$CMAKE_NO_TESTS
cd ..

if [ "$CMAKE_SHARED_LIBS" = 1 ]; then

mkdir __build_shared
cd __build_shared
cmake -DBoost_VERBOSE=1 ${CMAKE_BUILD_TESTING} -DCMAKE_INSTALL_PREFIX=iprefix \
    -DBUILD_SHARED_LIBS=ON ${CMAKE_OPTIONS} ../libs/json
cmake --build . --target install
ctest --output-on-failure --no-tests=$CMAKE_NO_TESTS

fi

elif [ "$DRONE_JOB_BUILDTYPE" == "cmake-subdirectory" ]; then

echo '==================================> INSTALL'

common_install

echo '==================================> COMPILE'

common_cmake

mkdir __build_static
cd __build_static
cmake ${CMAKE_BUILD_TESTING} ${CMAKE_OPTIONS} ../libs/json/test/cmake-subdir
cmake --build . --target check
cd ..

if [ "$CMAKE_SHARED_LIBS" = 1 ]; then

mkdir __build_shared
cd __build_shared
cmake ${CMAKE_BUILD_TESTING} -DBUILD_SHARED_LIBS=ON ${CMAKE_OPTIONS} \
    ../libs/json/test/cmake-subdir
cmake --build . --target check

fi

fi
