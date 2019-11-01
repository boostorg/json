//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
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
    is_control(char c) noexcept
    {
        return static_cast<unsigned char>(c) < 32;
    }

    static
    char
    hex_digit(char c) noexcept
    {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'A' && c <= 'F')
            return 10 + c - 'A';
        if (c >= 'a' && c <= 'f')
            return 10 + c - 'a';
        return -1;
    }
};

} // detail
} // json
} // boost

#endif
