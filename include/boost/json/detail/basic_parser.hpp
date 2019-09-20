//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_BASIC_PARSER_HPP
#define BOOST_JSON_DETAIL_BASIC_PARSER_HPP

namespace boost {
namespace json {
namespace detail {

struct parser_base
{
    static
    bool
    is_ws(char c) noexcept
    {
        return
            c == ' '  || c == '\t' ||
            c == '\r' || c == '\n';
    }

    static
    bool
    is_digit(char c) noexcept
    {
        return static_cast<unsigned char>(c - '0') < 10;
    }

    static
    bool
    is_control(char c)
    {
        return static_cast<unsigned char>(c) < 32;
    }
};

} // detail
} // json
} // boost

#endif
