//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_EXCEPT_HPP
#define BOOST_JSON_DETAIL_EXCEPT_HPP

#include <stdexcept>

namespace boost {
namespace json {
namespace detail {

inline
std::length_error
string_too_large_exception() noexcept
{
    return std::length_error(
        "string too large");
}

inline
std::length_error
key_too_large_exception() noexcept
{
    return std::length_error(
        "key too large");
}

inline
std::length_error
object_too_large_exception() noexcept
{
    return std::length_error(
        "object too large");
}

inline
std::length_error
stack_overflow_exception() noexcept
{
    return std::length_error(
        "stack overflow");
}

} // detail
} // json
} // boost

#endif
