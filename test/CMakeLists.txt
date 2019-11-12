#
# Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/vinniefalco/json
#

source_group (json FILES ${PROJECT_SOURCE_DIR}/include/boost/json.hpp)
source_group (json FILES ${PROJECT_SOURCE_DIR}/include/boost/pilfer.hpp)
source_group (TREE ${PROJECT_SOURCE_DIR}/include/boost/json PREFIX json FILES ${PROJECT_FILES})

GroupSources(test "/")

add_executable (json-tests
    ${BEAST_FILES}
    ${PROJECT_FILES}
    ${PROJECT_SOURCE_DIR}/include/boost/json.hpp
    ${PROJECT_SOURCE_DIR}/include/boost/pilfer.hpp
    ${PROJECT_SOURCE_DIR}/include/boost/json/json.natvis
    ${PROJECT_SOURCE_DIR}/src/src.cpp
    Jamfile
    test.hpp
    parse-vectors.hpp
    main.cpp
    array.cpp
    assign_string.cpp
    assign_vector.cpp
    basic_parser.cpp
    block_storage.cpp
    error.cpp
    json.cpp
    kind.cpp
    limits.cpp
    number.cpp
    object.cpp
    parser.cpp
    serializer.cpp
    storage.cpp
    storage_ptr.cpp
    string.cpp
    value.cpp
    ryu/d2s_intrinsics_test.cpp
    ryu/d2s_table_test.cpp
    ryu/d2s_test.cpp
    ryu/gtest.hpp
)

add_test(json-tests json-tests)
