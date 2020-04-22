#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/vinniefalco/json
#

cmake_minimum_required(VERSION 3.5...3.16)

set(BOOST_JSON_VERSION 1)
if(BOOST_SUPERPROJECT_VERSION)
    set(BOOST_JSON_VERSION ${BOOST_SUPERPROJECT_VERSION})
endif()

project(boost_json VERSION "${BOOST_JSON_VERSION}" LANGUAGES CXX)

option(BOOST_JSON_STANDALONE "Build boost::json as a static standalone library" FALSE)

set(BOOST_JSON_IS_ROOT_PROJECT OFF)

# this is the name of the cmake library target
# the cmake installer depends on this name being "json" if we are to allow
# cmake to automatically create the Config and name the library "Boost::json"
# however, the boost build tree requires it to be boost_json
set(BOOST_JSON_TARGET "boost_json")
set(BOOST_JSON_TARGET_ALIAS)
if(BOOST_JSON_STANDALONE)
    # Building standalone, out of Boost superproject tree, without Boost as dependency.
    set(BOOST_JSON_TARGET "json")
    set(BOOST_JSON_TARGET_ALIAS "boost_json")
elseif(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    #
    # Building as root project, out of Boost superproject tree, with Boost as dependency.
    # e.g. on Travis or other CI, or when producing Visual Studio Solution and Projects.
    #
    set(BOOST_JSON_IS_ROOT_PROJECT ON)
    get_filename_component(parent_dirname_ "${PROJECT_SOURCE_DIR}/.." ABSOLUTE)
    get_filename_component(parent_dirname_ "${parent_dirname_}" NAME)
    if(parent_dirname_ MATCHES "^[Ll][Ii][Bb][Ss]$")
        set(BOOST_JSON_BOOST_BUILD_TREE ON)
    else()
        set(BOOST_JSON_BOOST_BUILD_TREE OFF)
    endif()

    set(BOOST_JSON_TARGET "json")
    set(BOOST_JSON_TARGET_ALIAS "boost_json")
elseif(BOOST_SUPERPROJECT_VERSION)
    #
    # Building as part of Boost superproject tree, with Boost as dependency.
    #
else()
    #
    # Out-of-tree, non-standalone build
    #
    set(BOOST_JSON_TARGET "json")
    set(BOOST_JSON_TARGET_ALIAS "boost_json")
endif()



file(GLOB_RECURSE BOOST_JSON_HEADERS CONFIGURE_DEPENDS
    include/boost/*.hpp
    include/boost/*.ipp
    include/boost/*.natvis
)

set(BOOST_JSON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/src.cpp
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(${CMAKE_VERSION} VERSION_GREATER 3.7.2)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include/boost PREFIX "" FILES ${BOOST_JSON_HEADERS})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "" FILES ${BOOST_JSON_SOURCES})
endif()

if(BOOST_JSON_STANDALONE)
    # Building standalone, out of Boost superproject tree, without Boost as dependency.
    set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ standard (17 or higher is required)")
    if(CMAKE_CXX_STANDARD LESS 17)
        message(FATAL_ERROR "Boost.JSON requires C++17 in standalone mode, but CMAKE_CXX_STANDARD is '${CMAKE_CXX_STANDARD}'; please set it to 17 or higher")
    endif()
endif()

# TODO: For Boost superproject, do we want to support header-only mode?
#       Then, this needs to read `add_library(boost_json INTERFACE)`
#       and related settings need to be INTERFACE-ed as well.
add_library(${BOOST_JSON_TARGET} ${BOOST_JSON_HEADERS} ${BOOST_JSON_SOURCES})
if (BOOST_JSON_TARGET_ALIAS)
    add_library(${BOOST_JSON_TARGET_ALIAS} ALIAS ${BOOST_JSON_TARGET})
endif()
add_library(Boost::json ALIAS ${BOOST_JSON_TARGET})

target_compile_features(${BOOST_JSON_TARGET} PUBLIC cxx_constexpr)

# TODO: For Boost superproject, this may need to be INTERFACE setting
target_include_directories(${BOOST_JSON_TARGET} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>")

target_compile_definitions(${BOOST_JSON_TARGET} PUBLIC BOOST_JSON_NO_LIB=1)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(${BOOST_JSON_TARGET} PUBLIC BOOST_JSON_DYN_LINK=1)
else()
    target_compile_definitions(${BOOST_JSON_TARGET} PUBLIC BOOST_JSON_STATIC_LINK=1)
endif()

include(CTest)


if(BOOST_JSON_STANDALONE)
    # Building standalone, out of Boost superproject tree, without Boost as dependency.
    target_compile_definitions(boost_json PUBLIC BOOST_JSON_STANDALONE)
elseif(BOOST_JSON_IS_ROOT_PROJECT)
    #
    # Building as root project, out of Boost superproject tree, with Boost as dependency.
    # e.g. on Travis or other CI, or when producing Visual Studio Solution and Projects.
    #
    if(${CMAKE_VERSION} VERSION_LESS 3.16)
        message(FATAL_ERROR "Boost.JSON development requires CMake 3.16 or newer.")
    endif()

    if(BOOST_JSON_BOOST_BUILD_TREE)
        get_filename_component(BOOST_ROOT ../.. ABSOLUTE)
        target_include_directories(${BOOST_JSON_TARGET} PUBLIC ${BOOST_ROOT})
        target_link_directories(${BOOST_JSON_TARGET} PUBLIC ${BOOST_ROOT}/stage/lib)
    else()
        find_package(Boost REQUIRED COMPONENTS system)
        target_link_libraries(${BOOST_JSON_TARGET} PUBLIC Boost::system)
        include(cmake/make_cmake_installer.cmake)
        make_cmake_installer()
    endif()

elseif(BOOST_SUPERPROJECT_VERSION)
    #
    # Building as part of Boost superproject tree, with Boost as dependency.
    #
    # TODO: This CMake support for Boost.Json is currently experimental.
    #       This needs to be reviewed, tested
    target_link_libraries(${BOOST_JSON_TARGET}
        PUBLIC
            Boost::assert
            Boost::config
            Boost::core
            Boost::exception
            Boost::system
            Boost::utility
    )

    include(BoostInstall)
    boost_install(TARGETS ${BOOST_JSON_TARGET} HEADER_DIRECTORY include/)
else()
    #
    # Out-of-tree, non-standalone build
    #
    find_package(Boost COMPONENTS system)
    target_link_libraries(${BOOST_JSON_TARGET}
        PUBLIC
            Boost::system
    )
endif()

if (BUILD_TESTING)
    add_subdirectory(test)
endif()

if (BUILD_TESTING AND NOT BOOST_SUPERPROJECT_VERSION)
    add_subdirectory(bench)
    add_subdirectory(example)
endif()
