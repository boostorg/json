//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_NUMBER_IPP
#define BOOST_JSON_IMPL_NUMBER_IPP

#include <boost/json/number.hpp>
#include <boost/json/detail/math.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <ostream>
#include <cstdio>
#include <string>
#include <vector>

#include <boost/json/detail/ryu/ryu.hpp>

namespace boost {
namespace json {

//----------------------------------------------------------

/*  Reference work:

    https://www.ampl.com/netlib/fp/dtoa.c

    https://www.exploringbinary.com/fast-path-decimal-to-floating-point-conversion/

    https://kkimdev.github.io/posts/2018/06/15/IEEE-754-Floating-Point-Type-in-C++.html
*/

//----------------------------------------------------------

number::
number() noexcept
    : kind_(kind::type_int64)
{
    int64_ = 0;
}

//----------------------------------------------------------

number::
number(ieee_decimal const& dec) noexcept
{
    if(dec.exponent == 0)
    {
        if(! dec.sign)
        {
            assign_impl(dec.mantissa);
        }
        else if(dec.mantissa <= static_cast<unsigned long long>(
            (std::numeric_limits<long long>::max)()) + 1)
        {
            assign_impl(static_cast<
                long long>(dec.mantissa));
        }
        else
        {
            assign_impl(to_double(dec));
        }
    }
    else
    {
        auto const d = to_double(dec);
        if(! dec.sign)
        {
            auto v = static_cast<unsigned long long>(d);
            if(v == d)
                assign_impl(v);
            else
                assign_impl(d);
        }
        else
        {
            auto v = static_cast<long long>(d);
            if(v == d)
                assign_impl(v);
            else
                assign_impl(d);
        }
    }
}

//----------------------------------------------------------

bool
number::
is_int64() const noexcept
{
    switch(kind_)
    {
    default:
    case kind::type_int64:
        return true;

    case kind::type_uint64:
        return int64_ >= 0;

    case kind::type_double:
        return static_cast<long long>(
            double_) == double_;
    }
}

bool
number::
is_uint64() const noexcept
{
    switch(kind_)
    {
    default:
    case kind::type_int64:
        return int64_ >= 0;

    case kind::type_uint64:
        return true;

    case kind::type_double:
        return static_cast<
            unsigned long long>(
                double_) == double_;
    }
}

std::int_least64_t
number::
get_int64() const noexcept
{
    switch(kind_)
    {
    default:
    case kind::type_int64:
        return int64_;

    case kind::type_uint64:
        return static_cast<
            long long>(uint64_);

    case kind::type_double:
        return static_cast<
            long long>(double_);
    }
}

std::uint_least64_t
number::
get_uint64() const noexcept
{
    switch(kind_)
    {
    default:
    case kind::type_int64:
        return static_cast<
            unsigned long long>(int64_);

    case kind::type_uint64:
        return uint64_;

    case kind::type_double:
        return static_cast<
            unsigned long long>(double_);
    }
}

double
number::
get_double() const noexcept
{
    switch(kind_)
    {
    default:
    case kind::type_int64:
        return static_cast<double>(int64_);

    case kind::type_uint64:
        return static_cast<double>(uint64_);

    case kind::type_double:
        return double_;
    }
}

namespace detail {

inline
std::size_t
print_uint64(
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

    return dest - start;
}

inline
std::size_t
print_int64(
    char* dest, int64_t v) noexcept
{
    std::uint64_t ui = static_cast<
        std::uint64_t>(v);
    if(v >= 0)
        return print_uint64(dest, ui);
    *dest++ = '-';
    ui = ~ui + 1;
    return 1 + print_uint64(dest, ui);
}

} // detail

string_view
number::
print(
    char* buf,
    std::size_t /*buf_size*/) const noexcept
{
    std::size_t n;
    switch(kind_)
    {
    default:
    case kind::type_int64:
        n = detail::print_int64(buf, int64_);
        break;

    case kind::type_uint64:
        n = detail::print_uint64(buf, uint64_);
        break;

    case kind::type_double:
        n = detail::ryu::d2s_buffered_n(double_, buf);
        break;
    }
    return { buf, static_cast<
        std::size_t>(n) };
}

//----------------------------------------------------------

std::ostream&
operator<<(std::ostream& os, number const& n)
{
    char buf[number::max_string_chars];
    os << n.print(buf, sizeof(buf));
    return os;
}

bool
operator==(
    number const& lhs,
    number const& rhs) noexcept
{
    switch(lhs.kind_)
    {
    default:
    case number::kind::type_int64:
        return
            rhs.is_int64() &&
            lhs.get_int64() ==
                rhs.get_int64();

    case number::kind::type_uint64:
        return
            rhs.is_uint64() &&
            lhs.get_uint64() ==
                rhs.get_uint64();

    case number::kind::type_double:
        return
            lhs.get_double() ==
            rhs.get_double();
    }
}

bool
operator!=(
    number const& lhs,
    number const& rhs) noexcept
{
    return !(lhs == rhs);
}

} // json
} // boost

#endif
