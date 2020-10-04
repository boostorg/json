//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_NUMBER_CAST_HPP
#define BOOST_JSON_DETAIL_NUMBER_CAST_HPP

#include <boost/json/value.hpp>
#include <boost/json/error.hpp>
#include <limits>
#include <type_traits>

BOOST_JSON_NS_BEGIN
namespace detail {

template<class T>
using is_signed_integral =
    std::integral_constant<bool,
        std::is_signed<T>::value &&
        ! std::is_floating_point<T>::value>;

template<class T>
using is_unsigned_integral =
    std::integral_constant<bool,
        std::is_unsigned<T>::value &&
        ! std::is_same<T, bool>::value>;

template<class T>
T
number_cast(
    value const& jv,
    error_code& ec,
    typename std::enable_if<
        is_signed_integral<T>::value
            >::type* = 0) noexcept
{
    T result{};
    if(jv.kind() == kind::int64)
    {
        auto const i = jv.get_int64();
        if( i > (std::numeric_limits<T>::max)() ||
            i < (std::numeric_limits<T>::min)())
        {
            ec = error::not_exact;
        }
        else
        {
            result = static_cast<T>(i);
        }
    }
    else if(jv.kind() == kind::uint64)
    {
        auto const u = jv.get_uint64();
        if(u > static_cast<std::uint64_t>((
            std::numeric_limits<T>::max)()))
        {
            ec = error::not_exact;
        }
        else
        {
            result = static_cast<T>(u);
        }
    }
    else
    {
        BOOST_ASSERT(jv.kind() == kind::double_);
        auto const d = jv.get_double();
        if( d > (std::numeric_limits<T>::max)() ||
            d < (std::numeric_limits<T>::min)() ||
            static_cast<T>(d) != d)
        {
            ec = error::not_exact;
        }
        else
        {
            result = static_cast<T>(d);
        }
    }
    return result;
}

template<class T>
T
number_cast(
    value const& jv,
    error_code& ec,
    typename std::enable_if<
        is_unsigned_integral<T>::value
            >::type* = 0
    ) noexcept
{
    T result{};
    if(jv.kind() == kind::int64)
    {
        auto const i = jv.get_int64();
        if( i < 0 || static_cast<std::uint64_t>(i) >
            (std::numeric_limits<T>::max)())
        {
            ec = error::not_exact;
        }
        else
        {
            result = static_cast<T>(i);
        }
    }
    else if(jv.kind() == kind::uint64)
    {
        auto const u = jv.get_uint64();
        if(u > (std::numeric_limits<T>::max)())
        {
            ec = error::not_exact;
        }
        else
        {
            result = static_cast<T>(u);
        }
    }
    else
    {
        BOOST_ASSERT(jv.kind() == kind::double_);
        auto const d = jv.get_double();
        if( d < 0 ||
            d > (std::numeric_limits<T>::max)() ||
            static_cast<T>(d) != d)
        {
            ec = error::not_exact;
        }
        else
        {
            result = static_cast<T>(d);
        }
    }
    return result;
}

template<class T>
T
number_cast(
    value const& jv,
    error_code&,
    typename std::enable_if<
        std::is_floating_point<T>::value
            >::type* = 0
    ) noexcept
{
    if(jv.kind() == kind::int64)
        return static_cast<T>(jv.get_int64());

    if(jv.kind() == kind::uint64)
        return static_cast<T>(jv.get_uint64());

    BOOST_ASSERT(jv.kind() == kind::double_);
    return static_cast<T>(jv.get_double());
}

} // detail
BOOST_JSON_NS_END

#endif
