#!/bin/bash

set -ex

echo '==================================> BEFORE_INSTALL'

if  [ "$DRONE_BEFORE_INSTALL" == "gcc11" ]; then
    wget http://kayari.org/gcc-latest/gcc-latest.deb
    sudo dpkg -i gcc-latest.deb
    export CXX=/opt/gcc-latest/bin/g++
    export PATH=/opt/gcc-latest/bin:$PATH
    export LD_RUN_PATH=/opt/gcc-latest/lib64
fi

echo '==================================> INSTALL'

# Installing cmake with apt-get, so not required here:
# pip install --user cmake

echo '==================================> COMPILE'

export CXXFLAGS="-Wall -Wextra -Werror -std=c++17"
mkdir __build_17
cd __build_17
cmake -DBOOST_JSON_STANDALONE=1 ..
cmake --build .
ctest -V .
export CXXFLAGS="-Wall -Wextra -Werror -std=c++2a"
mkdir ../__build_2a
cd ../__build_2a
cmake -DBOOST_JSON_STANDALONE=1 ..
cmake --build .

if  [ "$CXX" == "clang++-12" ]; then
    ctest -VC .
else
    #default case:
    ctest -V .
fi

