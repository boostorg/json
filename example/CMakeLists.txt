#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/vinniefalco/json
#

GroupSources(example "/")

add_executable (pretty
    ${PROJECT_SOURCE_DIR}/src/src.cpp
    file.hpp
    pretty.cpp
)
set_property(TARGET pretty PROPERTY FOLDER "example")

add_executable (validate
    ${PROJECT_SOURCE_DIR}/src/src.cpp
    file.hpp
    validate.cpp
)
set_property(TARGET validate PROPERTY FOLDER "example")
