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

#include <boost/json/detail/ryu/ryu.hpp>

#include <cstring>

namespace boost {
namespace json {
namespace detail {

namespace {

/*  Reference work:

    https://www.ampl.com/netlib/fp/dtoa.c
    https://www.exploringbinary.com/fast-path-decimal-to-floating-point-conversion/
    https://kkimdev.github.io/posts/2018/06/15/IEEE-754-Floating-Point-Type-in-C++.html
*/

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

} // namespace

//------------------------------------------------

string_view
write_int64(
    char* temp,
    std::size_t size,
    std::int64_t v) noexcept
{
    BOOST_ASSERT( size >= max_number_chars );
    (void)size;

    std::uint64_t ui = static_cast<
        std::uint64_t>(v);
    unsigned n;
    if(v >= 0)
    {
        n = format_uint64(temp, ui);
    }
    else
    {
        *temp = '-';
        ui = ~ui + 1;
        n = 1 + format_uint64(temp + 1, ui);
    }

    return { temp, n };
}

string_view
write_uint64(
    char* temp,
    std::size_t size,
    std::uint64_t v) noexcept
{
    BOOST_ASSERT( size >= max_number_chars );
    (void)size;

    auto const n =
        format_uint64(temp, v);
    return { temp, n };
}

string_view
write_double(
    char* temp,
    std::size_t size,
    double v) noexcept
{
    BOOST_ASSERT( size >= max_number_chars );
    (void)size;

    unsigned const n =
        ryu::d2s_buffered_n(v, temp);
    return { temp, n };
}

} // detail
} // namespace json
} // namespace boost

#endif
