#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/vinniefalco/json
#

file(GLOB_RECURSE BOOST_JSON_TESTS_FILES CONFIGURE_DEPENDS Jamfile *.cpp *.hpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "" FILES ${BOOST_JSON_TESTS_FILES})
add_executable(tests ${BOOST_JSON_TESTS_FILES})
target_include_directories(tests PRIVATE .)
target_link_libraries(tests PRIVATE Boost::json)
add_test(NAME json-tests COMMAND tests)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "" FILES limits.cpp main.cpp)
add_executable(limits limits.cpp main.cpp)
message("BOOST_JSON_STANDALONE: ${BOOST_JSON_STANDALONE}")
message("BOOST_JSON_BOOST_BUILD_TREE: ${BOOST_JSON_BOOST_BUILD_TREE}")
message("BOOST_JSON_IS_ROOT_PROJECT: ${BOOST_JSON_IS_ROOT_PROJECT}")

if(BOOST_JSON_STANDALONE)
    target_include_directories(limits PRIVATE ../include)
    target_compile_definitions(limits PRIVATE BOOST_JSON_STANDALONE)
elseif(BOOST_JSON_BOOST_BUILD_TREE)
    target_include_directories(limits PRIVATE ${BOOST_ROOT})
elseif(BOOST_JSON_IS_ROOT_PROJECT)
    target_include_directories(limits PRIVATE ../include)
endif()
target_include_directories(limits PRIVATE .)
add_test(NAME json-limits COMMAND limits)
target_compile_definitions(limits PUBLIC
    BOOST_JSON_HEADER_ONLY=1
    BOOST_JSON_DYN_LINK=1
    BOOST_JSON_MAX_STRING_SIZE=1000
    BOOST_JSON_MAX_OBJECT_SIZE=30
    BOOST_JSON_MAX_ARRAY_SIZE=100
    BOOST_JSON_MAX_STACK_SIZE=1024
    BOOST_JSON_PARSER_BUFFER_SIZE=256
    )
