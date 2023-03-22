//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Peter Dimov (pdimov at gmail dot com),
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_IMPL_FORMAT_IPP
#define BOOST_JSON_DETAIL_IMPL_FORMAT_IPP

#include <boost/json/detail/config.hpp>
#include <boost/json/detail/ryu/ryu.hpp>
#include <limits>
#include <cstring>
#include <cstdint>
#include <cmath>

#ifndef BOOST_NO_CXX17_HDR_CHARCONV
#  include <charconv>
#  define BOOST_JSON_USE_CHARCONV
#endif 

// Narrowed down based on compiler support for integers vs floating point
// See: https://en.cppreference.com/w/cpp/compiler_support/17#C.2B.2B17_library_features
// P0067R5 - Elementary string conversions
#ifdef BOOST_JSON_USE_CHARCONV

#  if __GNUC__ >= 11
#    define BOOST_JSON_USE_INTEGER_CHARCONV
#    define BOOST_JSON_USE_FP_CHARCONV
#  elif __GNUC__ >= 8
#    define BOOST_JSON_USE_INTEGER_CHARCONV
#  endif // GCC

#  if __clang_major__ >= 14 && !defined(__APPLE_CC__)
#    define BOOST_JSON_USE_INTEGER_CHARCONV
#    define BOOST_JSON_USE_FP_CHARCONV
#  elif ( __clang_major__ >= 7 && !defined(__APPLE_CC__)) || __clang_major__ >= 10
#    define BOOST_JSON_USE_INTEGER_CHARCONV
#  endif // Clang

#  if _MSC_VER >= 1924
#    define BOOST_JSON_USE_INTEGER_CHARCONV
#    define BOOST_JSON_USE_FP_CHARCONV
#  elif _MSC_VER >= 1914
#    define BOOST_JSON_USE_INTEGER_CHARCONV
#  endif // MSVC

// If we don't know much about the compiler but it does provide the header <charconv>
// we will assume that it only provides the integer overloads until proven otherwise
#if !defined(BOOST_JSON_USE_INTEGER_CHARCONV) && !defined(BOOST_JSON_USE_FP_CHARCONV)
#  define BOOST_JSON_USE_INTEGER_CHARCONV
#endif

#endif

namespace boost {
namespace json {
namespace detail {

/*  Reference work:

    https://www.ampl.com/netlib/fp/dtoa.c
    https://www.exploringbinary.com/fast-path-decimal-to-floating-point-conversion/
    https://kkimdev.github.io/posts/2018/06/15/IEEE-754-Floating-Point-Type-in-C++.html
*/

#ifdef BOOST_JSON_USE_INTEGER_CHARCONV

unsigned
format_uint64(
    char* dest,
    uint64_t i) noexcept
{    
    static constexpr auto u64_max_chars = std::numeric_limits<uint64_t>::digits10 + 1;
    const auto r = std::to_chars(dest, dest + u64_max_chars, i);
    return static_cast<unsigned>(r.ptr - dest);
}

#else

inline char const* digits_lut() noexcept
{
    return
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
}

inline void format_four_digits( char * dest, unsigned v )
{
    std::memcpy( dest + 2, digits_lut() + (v % 100) * 2, 2 );
    std::memcpy( dest    , digits_lut() + (v / 100) * 2, 2 );
}

inline void format_two_digits( char * dest, unsigned v )
{
    std::memcpy( dest, digits_lut() + v * 2, 2 );
}

inline void format_digit( char * dest, unsigned v )
{
    *dest = static_cast<char>( v + '0' );
}

unsigned
format_uint64(
    char* dest,
    std::uint64_t v) noexcept
{
    if(v < 10)
    {
        *dest = static_cast<char>( '0' + v );
        return 1;
    }

    char buffer[ 24 ];

    char * p = buffer + 24;

    while( v >= 1000 )
    {
        p -= 4;
        format_four_digits( p, v % 10000 );
        v /= 10000;
    }

    if( v >= 10 )
    {
        p -= 2;
        format_two_digits( p, v % 100 );
        v /= 100;
    }

    if( v )
    {
        p -= 1;
        format_digit( p, static_cast<unsigned>(v) );
    }

    unsigned const n = static_cast<unsigned>( buffer + 24 - p );
    std::memcpy( dest, p, n );

    return n;
}

#endif // Use charconv

#ifdef BOOST_JSON_USE_INTEGER_CHARCONV
unsigned
format_int64(
    char* dest, int64_t i) noexcept
{    
    static constexpr auto i64_max_chars = std::numeric_limits<int64_t>::digits10 + 2;
    const auto r = std::to_chars(dest, dest + i64_max_chars, i);
    return static_cast<unsigned>(r.ptr - dest);
}
#else

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

#endif // Use charconv

#ifdef BOOST_JSON_USE_FP_CHARCONV

unsigned
format_double(
    char* dest, double d) noexcept
{
    // Special handling of 0 to maintain consistency across C++ versions
    // to_chars would return 0e+0 or -0e+0
    if (d == 0.0)
    {
        if (std::signbit(d))
        {
            std::memcpy(dest, "-0E0", 4);
            return 4U;
        }
        else
        {
            std::memcpy(dest, "0E0", 3);
            return 3U;
        }
    }

    const auto r = std::to_chars(dest, dest + max_number_chars, d, std::chars_format::scientific);
    return static_cast<unsigned>(r.ptr - dest);
}

#else

unsigned
format_double(
    char* dest, double d) noexcept
{
    return static_cast<unsigned>(
        ryu::d2s_buffered_n(d, dest));
}

#endif // Use charconv

} // detail
} // namespace json
} // namespace boost

#endif
