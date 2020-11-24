#!/bin/bash

set -ex

echo '==================================> BEFORE_INSTALL'

set -e
echo ">>>>>"
echo ">>>>> APT: REPO.."
echo ">>>>>"
sudo -E apt-add-repository -y "ppa:ubuntu-toolchain-r/test"
wget -O - https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS-2023.PUB | sudo apt-key add -
sudo -E apt-add-repository 'deb https://apt.repos.intel.com/oneapi all main'
sudo -E apt-get update
sudo -E apt-get -o Acquire::Retries=3 -yq --no-install-suggests --no-install-recommends install intel-oneapi-icc

echo '==================================> INSTALL'

source /opt/intel/oneapi/compiler/latest/env/vars.sh
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

cd $BOOST_ROOT/libs/$SELF
$BOOST_ROOT/libs/$SELF/ci/travis/build.sh
