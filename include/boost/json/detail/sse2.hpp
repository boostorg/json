//
// Copyright (c) 2019 Peter Dimov (pdimov at gmail dot com),
//                    Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_SSE2_HPP
#define BOOST_JSON_DETAIL_SSE2_HPP

#include <boost/json/config.hpp>
#ifdef BOOST_JSON_USE_SSE2
# include <emmintrin.h>
# include <xmmintrin.h>
# ifdef _MSC_VER
#  include <intrin.h>
# endif
#endif

namespace boost {
namespace json {
namespace detail {

#ifdef BOOST_JSON_USE_SSE2

inline
size_t
count_unescaped(
    char const* s,
    size_t n) noexcept
{
    __m128i const q1 = _mm_set1_epi8( '\x22' ); // '"'
    __m128i const q2 = _mm_set1_epi8( '\\' );
    __m128i const q3 = _mm_set1_epi8( 0x20 );
    __m128i const q4 = _mm_set1_epi8( -1 );

    char const * s0 = s;

    while( n >= 16 )
    {
        __m128i v1 = _mm_loadu_si128( (__m128i const*)s );

        __m128i v2 = _mm_cmpeq_epi8( v1, q1 );
        __m128i v3 = _mm_cmpeq_epi8( v1, q2 );
        __m128i v4 = _mm_and_si128(
            _mm_cmplt_epi8( v1, q3 ),
            _mm_cmpgt_epi8( v1, q4 ) ); // ch > -1 && ch < 0x20

        __m128i v5 = _mm_or_si128( v2, v3 );
        __m128i v6 = _mm_or_si128( v5, v4 );

        int w = _mm_movemask_epi8( v6 );

        if( w != 0 )
        {
            int m;
#if defined(__GNUC__) || defined(__clang__)
            m = __builtin_ffs( w ) - 1;
#else
            unsigned long index;
            _BitScanForward( &index, w );
            m = index;
#endif

            s += m;
            break;
        }

        s += 16;
        n -= 16;
    }

    return s - s0;
}

// assumes p..p+15 are valid
inline std::size_t count_digits( char const* p ) noexcept
{
    __m128i const q1 = _mm_set1_epi8( '0' );
    __m128i const q2 = _mm_set1_epi8( '9' );

    __m128i v1 = _mm_loadu_si128( (__m128i const*)p );

    v1 = _mm_or_si128(
            _mm_cmplt_epi8( v1, q1 ),
            _mm_cmpgt_epi8( v1, q2 ) );

    int m = _mm_movemask_epi8( v1 );

    std::size_t n;

    if( m == 0 )
    {
        n = 16;
    }
    else
    {
#if defined(__GNUC__) || defined(__clang__)
        n = __builtin_ffs( m ) - 1;
#else
        unsigned long index;
        _BitScanForward( &index, m );
        n = index;
#endif
    }

    return n;
}

#else

inline
std::size_t
count_unescaped(
    char const*,
    std::size_t) noexcept
{
    return 0;
}

// assumes p..p+15 are valid
inline std::size_t count_digits( char const* p ) noexcept
{
    std::size_t n = 0;

    for( ; n < 16; ++n )
    {
        unsigned char const d = *p++ - '0';
        if(d > 9) break;
    }

    return n;
}

#endif

inline uint64_t parse_unsigned( uint64_t r, char const * p, std::size_t n ) noexcept
{
    while( n >= 4 )
    {
        uint32_t v;
        memcpy( &v, p, 4 );

        v -= 0x30303030;

        unsigned w0 = v & 0xFF;
        unsigned w1 = (v >> 8) & 0xFF;
        unsigned w2 = (v >> 16) & 0xFF;
        unsigned w3 = (v >> 24);

        r = (((r * 10 + w0) * 10 + w1) * 10 + w2) * 10 + w3;

        p += 4;
        n -= 4;
    }

    while( n > 0 )
    {
        r = r * 10 + *p - '0';
        ++p;
        --n;
    }

    return r;
}

} // detail
} // json
} // boost

#endif
