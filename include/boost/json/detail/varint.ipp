//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_VARINT_IPP
#define BOOST_JSON_DETAIL_VARINT_IPP

#include <boost/json/detail/varint.hpp>
#include <utility>

namespace boost {
namespace json {
namespace detail {

int
varint_size(std::uint64_t value)
{
    int n = 1;
    while(value > 127)
    {
        ++n;
        value /= 128;
    }
    return n;
}

std::pair<std::uint64_t, int>
varint_read(void const* src)
{
    auto cp0 = reinterpret_cast<
        unsigned char const*>(src);
    auto cp = cp0;
    std::size_t value = 0;
    std::size_t factor = 1;
    while(*cp > 127)
    {
        value += (*cp++ & 0x7f) * factor;
        factor *= 128;
    }
    value += *cp++ * factor;
    return {value,
        static_cast<int>(cp - cp0)};
}

int
varint_write(
    void* dest,
    std::uint64_t value)
{
    auto cp0 = reinterpret_cast<
        unsigned char*>(dest);
    auto cp = cp0;
    while(value > 127)
    {
        *cp++ = static_cast<
            unsigned char>(value & 0x7f);
        value >>= 7;
    }
    *cp++ = static_cast<
        unsigned char>(value);
    return static_cast<int>(cp - cp0);
}

} // detail
} // json
} // boost

#endif
