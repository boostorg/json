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
    cbor_dom.cpp
    cbor_stack.cpp
    path.cpp
    pretty.cpp
    proxy.cpp
    validate.cpp
)

#

add_executable(cbor_dom
    cbor_dom.cpp
)
set_property(TARGET cbor_dom PROPERTY FOLDER "example")
target_link_libraries(cbor_dom PRIVATE Boost::json)

#

add_executable(cbor_stack
    cbor_stack.cpp
)
set_property(TARGET cbor_stack PROPERTY FOLDER "example")
target_link_libraries(cbor_stack PRIVATE Boost::json)

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
