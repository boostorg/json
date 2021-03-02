
echo '==================================> BEFORE_INSTALL'

echo '==================================> INSTALL'

REM Installing cmake with choco in the Dockerfile, so not required here:
REM choco install cmake

echo '==================================> COMPILE'

set CXXFLAGS="/std:c++17"
mkdir __build_17
cd __build_17
cmake -DBOOST_JSON_STANDALONE=1 ..
cmake --build .
ctest -V -C Debug .
set CXXFLAGS="/std:c++latest"
mkdir ..\__build_2a
cd ..\__build_2a
cmake -DBOOST_JSON_STANDALONE=1 ..
cmake --build .
ctest -V -C Debug .
