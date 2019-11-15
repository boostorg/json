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
    pretty.cpp
)
set_property(TARGET pretty PROPERTY FOLDER "example")

add_executable (allocator
    allocator.cpp
)
set_property(TARGET allocator PROPERTY FOLDER "example")
