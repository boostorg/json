//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_STRING_HPP
#define BOOST_JSON_STRING_HPP

#include <boost/beast/core/detail/config.hpp>
#include <boost/json/allocator.hpp>
#include <string>

namespace boost {
namespace beast {
namespace json {

class value;

/** The native type of string values
*/
using string =
    std::basic_string<
        char,
        std::char_traits<char>,
        allocator<char>>;

} // json
} // beast
} // boost

#endif
