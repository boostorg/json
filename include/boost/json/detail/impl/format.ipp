//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_IMPL_FORMAT_IPP
#define BOOST_JSON_DETAIL_IMPL_FORMAT_IPP

#include <boost/json/detail/ryu/ryu.hpp>
#include <cstdint>

namespace boost {
namespace json {
namespace detail {

/*  Reference work:

    https://www.ampl.com/netlib/fp/dtoa.c

    https://www.exploringbinary.com/fast-path-decimal-to-floating-point-conversion/

    https://kkimdev.github.io/posts/2018/06/15/IEEE-754-Floating-Point-Type-in-C++.html
*/

unsigned
format_uint64(
    char* dest,
    std::uint64_t value) noexcept
{
    auto const start = dest;
    char const* const lut =
        "00010203040506070809"
        "10111213141516171819"
        "20212223242526272829"
        "30313233343536373839"
        "40414243444546474849"
        "50515253545556575859"
        "60616263646566676869"
        "70717273747576777879"
        "80818283848586878889"
        "90919293949596979899";

    std::uint64_t const  t_8 = 100000000;
    std::uint64_t const  t_9 = t_8 * 10;
    std::uint64_t const t_10 = t_8 * 100;
    std::uint64_t const t_11 = t_8 * 1000;
    std::uint64_t const t_12 = t_8 * 10000;
    std::uint64_t const t_13 = t_8 * 100000;
    std::uint64_t const t_14 = t_8 * 1000000;
    std::uint64_t const t_15 = t_8 * 10000000;
    std::uint64_t const t_16 = t_8 * t_8;

    if(value < t_8)
    {
        std::uint32_t v = static_cast<std::uint32_t>(value);
        if(v < 10000)
        {
            std::uint32_t const d1 = (v / 100) << 1;
            std::uint32_t const d2 = (v % 100) << 1;

            if(v >= 1000)
                *dest++ = lut[d1];
            if(v >= 100)
                *dest++ = lut[d1 + 1];
            if(v >= 10)
                *dest++ = lut[d2];
            *dest++ = lut[d2 + 1];
        }
        else
        {
            std::uint32_t const b = v / 10000;
            std::uint32_t const c = v % 10000;
            std::uint32_t const d1 = (b / 100) << 1;
            std::uint32_t const d2 = (b % 100) << 1;
            std::uint32_t const d3 = (c / 100) << 1;
            std::uint32_t const d4 = (c % 100) << 1;

            if(value >= 10000000)
                *dest++ = lut[d1];
            if(value >= 1000000)
                *dest++ = lut[d1 + 1];
            if(value >= 100000)
                *dest++ = lut[d2];
            *dest++ = lut[d2 + 1];
            *dest++ = lut[d3];
            *dest++ = lut[d3 + 1];
            *dest++ = lut[d4];
            *dest++ = lut[d4 + 1];
        }
    }
    else if(value < t_16)
    {
        std::uint32_t const v0 =
            static_cast<std::uint32_t>(value / t_8);
        std::uint32_t const v1 =
            static_cast<std::uint32_t>(value % t_8);
        std::uint32_t const b0 = v0 / 10000;
        std::uint32_t const c0 = v0 % 10000;
        std::uint32_t const d1 = (b0 / 100) << 1;
        std::uint32_t const d2 = (b0 % 100) << 1;
        std::uint32_t const d3 = (c0 / 100) << 1;
        std::uint32_t const d4 = (c0 % 100) << 1;
        std::uint32_t const b1 = v1 / 10000;
        std::uint32_t const c1 = v1 % 10000;
        std::uint32_t const d5 = (b1 / 100) << 1;
        std::uint32_t const d6 = (b1 % 100) << 1;
        std::uint32_t const d7 = (c1 / 100) << 1;
        std::uint32_t const d8 = (c1 % 100) << 1;

        if(value >= t_15)
            *dest++ = lut[d1];
        if(value >= t_14)
            *dest++ = lut[d1 + 1];
        if(value >= t_13)
            *dest++ = lut[d2];
        if(value >= t_12)
            *dest++ = lut[d2 + 1];
        if(value >= t_11)
            *dest++ = lut[d3];
        if(value >= t_10)
            *dest++ = lut[d3 + 1];
        if(value >= t_9)
            *dest++ = lut[d4];

        *dest++ = lut[d4 + 1];
        *dest++ = lut[d5];
        *dest++ = lut[d5 + 1];
        *dest++ = lut[d6];
        *dest++ = lut[d6 + 1];
        *dest++ = lut[d7];
        *dest++ = lut[d7 + 1];
        *dest++ = lut[d8];
        *dest++ = lut[d8 + 1];
    }
    else
    {
        std::uint32_t const a =
            static_cast<uint32_t>(value / t_16);
        value %= t_16;
        if(a < 10)
            *dest++ = static_cast<char>(
                '0' + static_cast<char>(a));
        else if(a < 100)
        {
            std::uint32_t const i = a << 1;
            *dest++ = lut[i];
            *dest++ = lut[i + 1];
        }
        else if(a < 1000)
        {
            *dest++ = static_cast<char>(
                '0' + static_cast<char>(a / 100));

            std::uint32_t const i = (a % 100) << 1;
            *dest++ = lut[i];
            *dest++ = lut[i + 1];
        }
        else
        {
            std::uint32_t const i = (a / 100) << 1;
            std::uint32_t const j = (a % 100) << 1;
            *dest++ = lut[i];
            *dest++ = lut[i + 1];
            *dest++ = lut[j];
            *dest++ = lut[j + 1];
        }

        std::uint32_t const v0 =
            static_cast<uint32_t>(value / t_8);
        std::uint32_t const v1 =
            static_cast<uint32_t>(value % t_8);

        std::uint32_t const b0 =  v0 / 10000;
        std::uint32_t const c0 =  v0 % 10000;
        std::uint32_t const d1 = (b0 / 100) << 1;
        std::uint32_t const d2 = (b0 % 100) << 1;
        std::uint32_t const d3 = (c0 / 100) << 1;
        std::uint32_t const d4 = (c0 % 100) << 1;
        std::uint32_t const b1 =  v1 / 10000;
        std::uint32_t const c1 =  v1 % 10000;
        std::uint32_t const d5 = (b1 / 100) << 1;
        std::uint32_t const d6 = (b1 % 100) << 1;
        std::uint32_t const d7 = (c1 / 100) << 1;
        std::uint32_t const d8 = (c1 % 100) << 1;

        *dest++ = lut[d1];
        *dest++ = lut[d1 + 1];
        *dest++ = lut[d2];
        *dest++ = lut[d2 + 1];
        *dest++ = lut[d3];
        *dest++ = lut[d3 + 1];
        *dest++ = lut[d4];
        *dest++ = lut[d4 + 1];
        *dest++ = lut[d5];
        *dest++ = lut[d5 + 1];
        *dest++ = lut[d6];
        *dest++ = lut[d6 + 1];
        *dest++ = lut[d7];
        *dest++ = lut[d7 + 1];
        *dest++ = lut[d8];
        *dest++ = lut[d8 + 1];
    }

    return static_cast<unsigned>(dest - start);
}

unsigned
format_int64(
    char* dest, int64_t i) noexcept
{
    std::uint64_t ui = static_cast<
        std::uint64_t>(i);
    if(i >= 0)
        return format_uint64(dest, ui);
    *dest++ = '-';
    ui = ~ui + 1;
    return 1 + format_uint64(dest, ui);
}

unsigned
format_double(
    char* dest, double d) noexcept
{
    return static_cast<int>(
        ryu::d2s_buffered_n(d, dest));
}

} // detail
} // json
} // boost

#endif
