#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/vinniefalco/json
#

source_group("" FILES
    file.hpp
    pretty.cpp
    validate.cpp
)

add_executable(pretty
    file.hpp
    pretty.cpp
)
set_property(TARGET pretty PROPERTY FOLDER "example")
target_link_libraries(pretty PRIVATE boost_json)

add_executable(validate
    file.hpp
    validate.cpp
)
set_property(TARGET validate PROPERTY FOLDER "example")
target_link_libraries(validate PRIVATE boost_json)
