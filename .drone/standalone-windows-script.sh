#!/bin/bash

set -ex

echo '==================================> BEFORE_INSTALL'

echo '==================================> INSTALL'

# Installing cmake with choco in the Dockerfile, so not required here:
# choco install cmake

echo '==================================> COMPILE'

export CXXFLAGS="/std:c++17"
mkdir __build_17
cd __build_17
cmake -DBOOST_JSON_STANDALONE=1 ..
cmake --build .
ctest -V -C Debug .
export CXXFLAGS="/std:c++latest"
mkdir ../__build_2a
cd ../__build_2a
cmake -DBOOST_JSON_STANDALONE=1 ..
cmake --build .
ctest -V -C Debug .
