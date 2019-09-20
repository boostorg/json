//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_ASSIGN_STRING_HPP
#define BOOST_JSON_ASSIGN_STRING_HPP

#include <boost/beast/core/detail/config.hpp>
#include <boost/json/error.hpp>
#include <boost/json/value.hpp>
#include <string>
#include <type_traits>

namespace boost {
namespace beast {
namespace json {

template<class Allocator>
void
from_json(
    std::basic_string<
        char,
        std::char_traits<char>,
        Allocator>& t,
    value const& v)
{
    if(! v.is_string())
        throw system_error(
            error::expected_string);
    auto& s= v.as_string();
    t.assign(s.data(), s.size());
}

} // json
} // beast
} // boost

#endif
