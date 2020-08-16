//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_MEMORY_RESOURCE_HPP
#define BOOST_JSON_MEMORY_RESOURCE_HPP

#include <boost/json/detail/config.hpp>
#ifndef BOOST_JSON_STANDALONE
# include <boost/container/pmr/memory_resource.hpp>
#else
# include <memory_resource>
#endif

namespace boost {
namespace json {

#ifdef BOOST_JSON_DOCS

/** The type of memory resource used by the library.

    This type alias is set depending
    on how the library is configured:

    @par Use with Boost

    If the macro `BOOST_JSON_STANDALONE` is
    not defined, this type will be an alias
    for `boost::container::pmr::memory_resource`.
    Compiling a program using the library will
    require Boost, and a compiler conforming
    to C++11 or later.

    @par Use without Boost

    If the macro `BOOST_JSON_STANDALONE` is
    defined, this type will be an alias
    for `std::pmr::memory_resource`.
    Compiling a program using the library will
    require only a compiler conforming to C++17
    or later.
*/
using memory_resource = __see_below__;

#elif ! defined(BOOST_JSON_STANDALONE)

using memory_resource = boost::container::pmr::memory_resource;

#else

using memory_resource = std::pmr::memory_resource;

#endif

} // json
} // boost

#endif
