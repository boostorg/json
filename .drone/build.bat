@ECHO OFF
setlocal enabledelayedexpansion

IF NOT DEFINED B2_CI_VERSION (
    echo You need to set B2_CI_VERSION in your CI script
    exit /B 1
)

PATH=%ADDPATH%%PATH%

SET B2_TOOLCXX=toolset=%B2_TOOLSET%

IF DEFINED B2_CXXSTD (SET B2_CXXSTD=cxxstd=%B2_CXXSTD%)
IF DEFINED B2_CXXFLAGS (SET B2_CXXFLAGS=cxxflags=%B2_CXXFLAGS%)
IF DEFINED B2_DEFINES (SET B2_DEFINES=define=%B2_DEFINES%)
IF DEFINED B2_ADDRESS_MODEL (SET B2_ADDRESS_MODEL=address-model=%B2_ADDRESS_MODEL%)
IF DEFINED B2_LINK (SET B2_LINK=link=%B2_LINK%)
IF DEFINED B2_VARIANT (SET B2_VARIANT=variant=%B2_VARIANT%)

cd %BOOST_ROOT%

IF DEFINED SCRIPT (
    call libs\%SELF%\%SCRIPT%
) ELSE (
    set SELF_S=%SELF:\=/%
    REM Echo the complete build command to the build log
    ECHO b2 --abbreviate-paths libs/!SELF_S!/test libs/!SELF_S!/example %B2_TOOLCXX% %B2_CXXSTD% %B2_CXXFLAGS% %B2_DEFINES% %B2_THREADING% %B2_ADDRESS_MODEL% %B2_LINK% %B2_VARIANT% -j3
    REM Now go build...
    b2 --abbreviate-paths libs/!SELF_S!/test libs/!SELF_S!/example %B2_TOOLCXX% %B2_CXXSTD% %B2_CXXFLAGS% %B2_DEFINES% %B2_THREADING% %B2_ADDRESS_MODEL% %B2_LINK% %B2_VARIANT% -j3
)