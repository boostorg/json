//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_NUMBER_IPP
#define BOOST_JSON_IMPL_NUMBER_IPP

#include <boost/json/number.hpp>
#include <boost/beast/core/static_string.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <ostream>
#include <cstdio>
#include <string>
#include <vector>

namespace boost {
namespace json {

//------------------------------------------------------------------------------

struct number::pow10
{
    std::size_t
    size() const noexcept
    {
        return size_;
    }

    mantissa_type const*
    begin() const noexcept
    {
        return begin_;
    }

    mantissa_type const*
    end() const noexcept
    {
        return end_;
    }

    mantissa_type
    operator[](
        exponent_type n) const
    {
        return begin_[n];
    }
        
    static
    pow10 const&
    get() noexcept
    {
        struct pow10_impl : pow10
        {
            pow10_impl()
            {
                static mantissa_type constexpr list[] = {
                    1ULL,
                    10ULL,
                    100ULL,
                    1000ULL,
                    10000ULL,
                    100000ULL,
                    1000000ULL,
                    10000000ULL,
                    100000000ULL,
                    1000000000ULL,
                    10000000000ULL,
                    100000000000ULL,
                    1000000000000ULL,
                    10000000000000ULL,
                    100000000000000ULL,
                    1000000000000000ULL,
                    10000000000000000ULL,
                    100000000000000000ULL,
                    1000000000000000000ULL,
                    10000000000000000000ULL
                };
                size_ = std::extent<
                    decltype(list)>::value;
                begin_ = &list[0];
                end_ = &list[size_];
            }
        };

        static pow10_impl const tab;
        return tab;
    }

protected:
    std::size_t size_;
    mantissa_type const* begin_;
    mantissa_type const* end_;
};

//------------------------------------------------------------------------------

number::
number(
    mantissa_type mant,
    exponent_type exp,
    bool sign) noexcept
{
    auto const as_double =
        [&]
        {
            double d =
                static_cast<double>(mant) *
                std::pow(10, exp);
            if(sign)
                d *= -1;
            return d;
        };

    if(exp == 0)
    {
        if(! sign)
        {
            assign_unsigned(mant);
        }
        else if(mant <= static_cast<unsigned long long>(
            (std::numeric_limits<long long>::max)()) + 1)
        {
            assign_signed(static_cast<long long>(mant));
        }
        else
        {
            assign_double(as_double());
        }
    }
    else
    {
        auto const d = as_double();
        if(! sign)
        {
            auto v = static_cast<unsigned long long>(d);
            if(v == d)
                assign_unsigned(v);
            else
                assign_double(d);
        }
        else
        {
            auto v = static_cast<long long>(d);
            if(v == d)
                assign_signed(v);
            else
                assign_double(d);
        }
    }
}

//------------------------------------------------------------------------------

number::
number(short i) noexcept
{
    assign_signed(i);
}

number::
number(int i) noexcept
{
    assign_signed(i);
}

number::
number(long i) noexcept
{
    assign_signed(i);
}

number::
number(long long i) noexcept
{
    assign_signed(i);
}

number::
number(unsigned short i) noexcept
{
    assign_unsigned(i);
}

number::
number(unsigned int i) noexcept
{
    assign_unsigned(i);
}

number::
number(unsigned long i) noexcept
{
    assign_unsigned(i);
}

number::
number(unsigned long long i) noexcept
{
    assign_unsigned(i);
}

number::
number(float f) noexcept
{
    assign_double(f);
}

number::
number(double f) noexcept
{
    assign_double(f);
}

//------------------------------------------------------------------------------

bool
number::
is_int64() const noexcept
{
    switch(k_)
    {
    case type_int64:
        return true;

    case type_uint64:
        return int64_ >= 0;

    case type_double:
        return static_cast<long long>(
            double_) == double_;

    default:
    case type_ieee:
        break;
    }
    return false;
}

bool
number::
is_uint64() const noexcept
{
    switch(k_)
    {
    case type_int64:
        return int64_ >= 0;

    case type_uint64:
        return true;

    case type_double:
        return static_cast<
            unsigned long long>(
                double_) == double_;

    default:
    case type_ieee:
        break;
    }
    return false;
}

std::int_least64_t
number::
get_int64() const noexcept
{
    switch(k_)
    {
    case type_int64:
        return int64_;

    case type_uint64:
        return static_cast<
            long long>(uint64_);

    case type_double:
        return static_cast<
            long long>(double_);

    default:
    case type_ieee:
        break;
    }
    return 0;
}

std::uint_least64_t
number::
get_uint64() const noexcept
{
    switch(k_)
    {
    case type_int64:
        return static_cast<
            unsigned long long>(int64_);

    case type_uint64:
        return uint64_;

    case type_double:
        return static_cast<
            unsigned long long>(double_);

    default:
    case type_ieee:
        break;
    }
    return 0;
}

double
number::
get_double() const noexcept
{
    switch(k_)
    {
    case type_int64:
        return static_cast<double>(int64_);

    case type_uint64:
        return static_cast<double>(uint64_);

    case type_double:
        return double_;

    default:
    case type_ieee:
        break;
    }
    return 0;
}

string_view
number::
print(
    char* buf,
    std::size_t buf_size) const noexcept
{
    int n;
    switch(k_)
    {
    case type_int64:
        n = snprintf(buf, buf_size,
            "%lld", int64_);
        break;

    case type_uint64:
        n = snprintf(buf, buf_size,
            "%llu", uint64_);
        break;

    case type_double:
        n = snprintf(buf, buf_size,
            "%e", double_);
        break;

    default:
    case type_ieee:
        n = snprintf(buf, buf_size,
            "_unimpl");
        break;
    }
    return { buf, static_cast<
        std::size_t>(n) };
}

//------------------------------------------------------------------------------

void
number::
assign_signed(long long i) noexcept
{
    k_ = type_int64;
    int64_ = i;
}

void
number::
assign_unsigned(unsigned long long i) noexcept
{
    k_ = type_uint64;
    uint64_ = i;
}

void
number::
assign_double(double f) noexcept
{
    k_ = type_double;
    double_ = f;
}

//------------------------------------------------------------------------------

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
    switch(lhs.k_)
    {
    case number::type_int64:
        return
            rhs.is_int64() &&
            lhs.get_int64() ==
                rhs.get_int64();

    case number::type_uint64:
        return
            rhs.is_uint64() &&
            lhs.get_uint64() ==
                rhs.get_uint64();

    case number::type_double:
        return
            lhs.get_double() ==
            rhs.get_double();

    default:
    case number::type_ieee:
        break;
    }
    return false;
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
