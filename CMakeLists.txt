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

if (BOOST_SUPERPROJECT_VERSION)
    project(boost_json VERSION "${BOOST_SUPERPROJECT_VERSION}" LANGUAGES CXX)
else()
    project(boost_json VERSION "${BOOST_JSON_VERSION}" LANGUAGES CXX)
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

# TODO: For Boost superproject, do we want to support header-only mode?
#       Then, this needs to read `add_library(boost_json INTERFACE)`
#       and related settings need to be INTERFACE-ed as well.
add_library(boost_json ${BOOST_JSON_HEADERS} ${BOOST_JSON_SOURCES})
add_library(Boost::json ALIAS boost_json)

target_compile_features(boost_json PUBLIC cxx_constexpr)

# TODO: For Boost superproject, this may need to be INTERFACE setting
target_include_directories(boost_json PUBLIC include)

target_compile_definitions(boost_json PUBLIC BOOST_JSON_NO_LIB=1)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(boost_json PUBLIC BOOST_JSON_DYN_LINK=1)
else()
    target_compile_definitions(boost_json PUBLIC BOOST_JSON_STATIC_LINK=1)
endif()

include(CTest)

option(BOOST_JSON_STANDALONE "Build boost::json as a static standalone library" FALSE)

if(BOOST_JSON_STANDALONE)
    #
    # Building standalone, out of Boost superproject tree, without Boost as dependency.
    #
    target_compile_features(boost_json PUBLIC cxx_std_17)
    target_compile_definitions(boost_json PUBLIC BOOST_JSON_STANDALONE)
elseif(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    #
    # Building as root project, out of Boost superproject tree, with Boost as dependency.
    # e.g. on Travis or other CI, or when producing Visual Studio Solution and Projects.
    #
    if(${CMAKE_VERSION} VERSION_LESS 3.16)
        message(FATAL_ERROR "Boost.JSON development requires CMake 3.16 or newer.")
    endif()

    if (BOOST_JSON_FIND_BOOST)
        find_package(Boost REQUIRED COMPONENTS system)
        target_link_libraries(boost_json PUBLIC Boost::system)
    else()
        get_filename_component(BOOST_ROOT ../.. ABSOLUTE)
        target_include_directories(boost_json PUBLIC ${BOOST_ROOT})
        target_link_directories(boost_json PUBLIC ${BOOST_ROOT}/stage/lib)
    endif()
elseif(BOOST_SUPERPROJECT_VERSION)
    #
    # Building as part of Boost superproject tree, with Boost as dependency.
    #
    # TODO: This CMake support for Boost.Json is currently experimental.
    #       This needs to be reviewed, tested
    target_link_libraries(boost_json
        PUBLIC
            Boost::assert
            Boost::config
            Boost::core
            Boost::exception
            Boost::system
            Boost::utility
    )

    include(BoostInstall)
    boost_install(TARGETS boost_json HEADER_DIRECTORY include/)
else()
    #
    # Out-of-tree, non-standalone build
    #
    find_package(Boost COMPONENTS system)
    target_link_libraries(boost_json
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
