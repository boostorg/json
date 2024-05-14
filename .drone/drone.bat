
@ECHO ON
setlocal enabledelayedexpansion

set TRAVIS_OS_NAME=windows

IF "!DRONE_BRANCH!" == "" (
  for /F %%i in ("!GITHUB_REF!") do @set TRAVIS_BRANCH=%%~nxi
) else (
  SET TRAVIS_BRANCH=!DRONE_BRANCH!
)

if "%DRONE_JOB_BUILDTYPE%" == "boost" (

echo "Running boost job"
echo '==================================> INSTALL'
REM there seems to be some problem with b2 bootstrap on Windows
REM when CXX env variable is set
SET "CXX="

git clone https://github.com/boostorg/boost-ci.git boost-ci-cloned --depth 1
cp -prf boost-ci-cloned/ci .
rm -rf boost-ci-cloned
REM source ci/travis/install.sh
REM The contents of install.sh below:

for /F %%i in ("%DRONE_REPO%") do @set SELF=%%~nxi
SET BOOST_CI_TARGET_BRANCH=!TRAVIS_BRANCH!
SET BOOST_CI_SRC_FOLDER=%cd%

call ci\common_install.bat

echo '==================================> COMPILE'

set B2_TARGETS=libs/!SELF!/test libs/!SELF!/example
call !BOOST_ROOT!\libs\!SELF!\ci\build.bat

) else if "%DRONE_JOB_BUILDTYPE%" == "cmake-superproject" (

echo "Running cmake superproject job"
echo '==================================> INSTALL'
REM there seems to be some problem with b2 bootstrap on Windows
REM when CXX env variable is set
SET "CXX="

git clone https://github.com/boostorg/boost-ci.git boost-ci-cloned --depth 1
cp -prf boost-ci-cloned/ci .
rm -rf boost-ci-cloned
REM source ci/travis/install.sh
REM The contents of install.sh below:

for /F %%i in ("%DRONE_REPO%") do @set SELF=%%~nxi
SET BOOST_CI_TARGET_BRANCH=!TRAVIS_BRANCH!
SET BOOST_CI_SRC_FOLDER=%cd%

call ci\common_install.bat

echo '==================================> COMPILE'

if "!CMAKE_NO_TESTS!" == "" (
    SET CMAKE_NO_TESTS=error
)
if "!CMAKE_NO_TESTS!" == "error" (
    SET CMAKE_BUILD_TESTING=-DBUILD_TESTING=ON
)


cd ../../

mkdir __build_static && cd __build_static
cmake -DBoost_VERBOSE=1 !CMAKE_BUILD_TESTING! -DCMAKE_INSTALL_PREFIX=iprefix ^
-DBOOST_INCLUDE_LIBRARIES=!SELF! !CMAKE_OPTIONS! ..

cmake --build . --target install --config Debug
if NOT "!CMAKE_BUILD_TESTING!" == "" (
    cmake --build . --target tests --config Debug
)
ctest --output-on-failure --no-tests=error -R boost_!SELF! -C Debug

cmake --build . --target install --config Release
if NOT "!CMAKE_BUILD_TESTING!" == "" (
    cmake --build . --target tests --config Release
)
ctest --output-on-failure --no-tests=error -R boost_!SELF! -C Release
cd ..


if "!CMAKE_SHARED_LIBS!" == "" (
    SET CMAKE_SHARED_LIBS=1
)
if "!CMAKE_SHARED_LIBS!" == "1" (

mkdir __build_shared && cd __build_shared
cmake -DBoost_VERBOSE=1 !CMAKE_BUILD_TESTING! -DCMAKE_INSTALL_PREFIX=iprefix ^
-DBOOST_INCLUDE_LIBRARIES=!SELF! -DBUILD_SHARED_LIBS=ON !CMAKE_OPTIONS! ..

cmake --build . --config Debug
cmake --build . --target install --config Debug
if NOT "!CMAKE_BUILD_TESTING!" == "" (
    cmake --build . --target tests --config Debug
)
ctest --output-on-failure --no-tests=error -R boost_!SELF! -C Debug

cmake --build . --config Release
cmake --build . --target install --config Release
if NOT "!CMAKE_BUILD_TESTING!" == "" (
    cmake --build . --target tests --config Release
)
ctest --output-on-failure --no-tests=error -R boost_!SELF! -C Release

)

) else if "%DRONE_JOB_BUILDTYPE%" == "cmake-mainproject" (

echo "Running cmake main project job"
echo '==================================> INSTALL'
REM there seems to be some problem with b2 bootstrap on Windows
REM when CXX env variable is set
SET "CXX="

git clone https://github.com/boostorg/boost-ci.git boost-ci-cloned --depth 1
cp -prf boost-ci-cloned/ci .
rm -rf boost-ci-cloned
REM source ci/travis/install.sh
REM The contents of install.sh below:

for /F %%i in ("%DRONE_REPO%") do @set SELF=%%~nxi
SET BOOST_CI_TARGET_BRANCH=!TRAVIS_BRANCH!
SET BOOST_CI_SRC_FOLDER=%cd%

call ci\common_install.bat

echo '==================================> COMPILE'

if "!CMAKE_NO_TESTS!" == "" (
    SET CMAKE_NO_TESTS=error
)
if "!CMAKE_NO_TESTS!" == "error" (
    SET CMAKE_BUILD_TESTING=-DBUILD_TESTING=ON
)


cd ../../

mkdir __build_static && cd __build_static
cmake -DBoost_VERBOSE=1 !CMAKE_BUILD_TESTING! -DCMAKE_INSTALL_PREFIX=iprefix ^
!CMAKE_OPTIONS! ../libs/json
cmake --build . --target install --config Debug
ctest --output-on-failure --no-tests=error -R boost_!SELF! -C Debug

cmake --build . --target install --config Release
ctest --output-on-failure --no-tests=error -R boost_!SELF! -C Release

) else if "%DRONE_JOB_BUILDTYPE%" == "cmake-subdirectory" (

echo "Running cmake subdirectory job"
echo '==================================> INSTALL'
REM there seems to be some problem with b2 bootstrap on Windows
REM when CXX env variable is set
SET "CXX="

git clone https://github.com/boostorg/boost-ci.git boost-ci-cloned --depth 1
cp -prf boost-ci-cloned/ci .
rm -rf boost-ci-cloned
REM source ci/travis/install.sh
REM The contents of install.sh below:

for /F %%i in ("%DRONE_REPO%") do @set SELF=%%~nxi
SET BOOST_CI_TARGET_BRANCH=!TRAVIS_BRANCH!
SET BOOST_CI_SRC_FOLDER=%cd%

call ci\common_install.bat

echo '==================================> COMPILE'

if "!CMAKE_NO_TESTS!" == "" (
    SET CMAKE_NO_TESTS=error
)
if "!CMAKE_NO_TESTS!" == "error" (
    SET CMAKE_BUILD_TESTING=-DBUILD_TESTING=ON
)


cd ../../

mkdir __build_static && cd __build_static
cmake !CMAKE_BUILD_TESTING! !CMAKE_OPTIONS! ../libs/json/test/cmake-subdir
cmake --build . --target check --config Debug
cmake --build . --target check --config Release

)
