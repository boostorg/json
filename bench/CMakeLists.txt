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

GroupSources(bench "/")

if (MSVC)
    if (CMAKE_BUILD_TYPE EQUAL "Debug")
    else()
        add_compile_options(
            /GL         # Whole program optimization
            /Ob2        # inline any suitable
            )

        add_link_options(
            /LTCG       # Link Time Code Generation
            )
    endif()
endif()

add_executable (bench
    ${BEAST_FILES}
    ${PROJECT_FILES}
    ${PROJECT_SOURCE_DIR}/include/boost/json.hpp
    ${PROJECT_SOURCE_DIR}/include/boost/pilfer.hpp
    Jamfile
    bench.cpp
)

add_test(json-tests json-tests)
