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
~number()
{
}

number::
number() noexcept
    : sp_(default_storage())
    , kind_(kind::type_int64)
{
    int64_ = 0;
}

number::
number(storage_ptr sp) noexcept
    : sp_(std::move(sp))
    , kind_(kind::type_int64)
{
    int64_ = 0;
}

number::
number(pilfered<number> p) noexcept
    : sp_(std::move(p.get().sp_))
    , kind_(p.get().kind_)
{
    auto& other = p.get();
    switch(kind_)
    {
    default:
    case kind::type_double:
        double_ = other.double_;
        break;

    case kind::type_int64:
        int64_ = other.int64_;
        break;

    case kind::type_uint64:
        uint64_ = other.uint64_;
        break;
    }
}

number::
number(number const& other)
    : sp_(other.sp_)
    , kind_(other.kind_)
{
    switch(kind_)
    {
    default:
    case kind::type_double:
        double_ = other.double_;
        break;

    case kind::type_int64:
        int64_ = other.int64_;
        break;

    case kind::type_uint64:
        uint64_ = other.uint64_;
        break;
    }
}

number::
number(
    number const& other,
    storage_ptr sp)
    : sp_(std::move(sp))
    , kind_(other.kind_)
{
    switch(kind_)
    {
    default:
    case kind::type_double:
        double_ = other.double_;
        break;

    case kind::type_int64:
        int64_ = other.int64_;
        break;

    case kind::type_uint64:
        uint64_ = other.uint64_;
        break;
    }
}

number::
number(number&& other)
    : sp_(other.sp_)
    , kind_(other.kind_)
{
    switch(kind_)
    {
    default:
    case kind::type_double:
        double_ = other.double_;
        break;

    case kind::type_int64:
        int64_ = other.int64_;
        break;

    case kind::type_uint64:
        uint64_ = other.uint64_;
        break;
    }
}

number::
number(
    number&& other,
    storage_ptr sp)
    : sp_(std::move(sp))
    , kind_(other.kind_)
{
    switch(kind_)
    {
    default:
    case kind::type_double:
        double_ = other.double_;
        break;

    case kind::type_int64:
        int64_ = other.int64_;
        break;

    case kind::type_uint64:
        uint64_ = other.uint64_;
        break;
    }
}

number&
number::
operator=(number const& other)
{
    kind_ = other.kind_;
    switch(kind_)
    {
    default:
    case kind::type_double:
        double_ = other.double_;
        break;

    case kind::type_int64:
        int64_ = other.int64_;
        break;

    case kind::type_uint64:
        uint64_ = other.uint64_;
        break;
    }
    return *this;
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

string_view
number::
print(
    char* buf,
    std::size_t buf_size) const noexcept
{
    int n;
    switch(kind_)
    {
    default:
    case kind::type_int64:
        n = snprintf(buf, buf_size,
            "%lld", int64_);
        break;

    case kind::type_uint64:
        n = snprintf(buf, buf_size,
            "%llu", uint64_);
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
