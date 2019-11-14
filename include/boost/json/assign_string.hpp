//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_ASSIGN_STRING_HPP
#define BOOST_JSON_ASSIGN_STRING_HPP

#include <boost/json/config.hpp>
#include <boost/json/error.hpp>
#include <boost/json/value.hpp>
#include <string>
#include <type_traits>

namespace boost {
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
    auto& s= v.as_string();
    t.assign(s.data(), s.size());
}

} // json
} // boost

#endif
