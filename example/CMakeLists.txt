#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/boostorg/json
#

source_group("" FILES
    file.hpp
    parse_into.cpp
    path.cpp
    pretty.cpp
    proxy.cpp
    validate.cpp
)

#

add_executable(parse_into
    parse_into.cpp
)
set_property(TARGET parse_into PROPERTY FOLDER "example")
target_link_libraries(parse_into PRIVATE Boost::json)

#

add_executable(path
    path.cpp
)
set_property(TARGET path PROPERTY FOLDER "example")
target_link_libraries(path PRIVATE Boost::json)

#

add_executable(pretty
    file.hpp
    pretty.cpp
)
set_property(TARGET pretty PROPERTY FOLDER "example")
target_link_libraries(pretty PRIVATE Boost::json)

#

add_executable(proxy
    file.hpp
    proxy.cpp
)
set_property(TARGET proxy PROPERTY FOLDER "example")
target_link_libraries(proxy PRIVATE Boost::json)

#

add_executable(validate
    file.hpp
    validate.cpp
)
set_property(TARGET validate PROPERTY FOLDER "example")
target_link_libraries(validate PRIVATE Boost::json)
