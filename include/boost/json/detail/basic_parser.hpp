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

    static
    int
    utf8_encode(
        char* dest,
        unsigned long cp)
    {
        if(cp < 0x80)
        {
            dest[0] = static_cast<char>(cp);
            return 1;
        }

        if(cp < 0x800)
        {
            dest[0] = static_cast<char>( (cp >> 6)          | 0xc0);
            dest[1] = static_cast<char>( (cp & 0x3f)        | 0x80);
            return 2;
        }

        if(cp < 0x10000)
        {
            dest[0] = static_cast<char>( (cp >> 12)         | 0xe0);
            dest[1] = static_cast<char>(((cp >> 6) & 0x3f)  | 0x80);
            dest[2] = static_cast<char>( (cp       & 0x3f)  | 0x80);
            return 3;
        }

        {
            dest[0] = static_cast<char>( (cp >> 18)         | 0xf0);
            dest[1] = static_cast<char>(((cp >> 12) & 0x3f) | 0x80);
            dest[2] = static_cast<char>(((cp >> 6)  & 0x3f) | 0x80);
            dest[3] = static_cast<char>( (cp        & 0x3f) | 0x80);
            return 4;
        }
    }

};

} // detail
} // json
} // boost

#endif
