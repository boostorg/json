//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_SSE2_HPP
#define BOOST_JSON_DETAIL_SSE2_HPP

#include <boost/json/detail/config.hpp>
#ifdef BOOST_JSON_USE_SSE2
#include <emmintrin.h>
#include <xmmintrin.h>
#endif

namespace boost {
namespace json {
namespace detail {

#ifdef BOOST_JSON_USE_SSE2
inline
unsigned long long
count_unescaped(
    char const* s,
    unsigned long long n) noexcept
{
    __m128i const q1 = _mm_set1_epi8( '"' );
    __m128i const q2 = _mm_set1_epi8( '\\' );
    __m128i const q3 = _mm_set1_epi8( 0x20 );

    char const * s0 = s;

    while( n >= 16 )
    {
        __m128i v1 = _mm_loadu_si128( (__m128i const*)s );

        __m128i v2 = _mm_cmpeq_epi8( v1, q1 );
        __m128i v3 = _mm_cmpeq_epi8( v1, q2 );
        __m128i v4 = _mm_cmplt_epi8( v1, q3 );

        __m128i v5 = _mm_or_si128( v2, v3 );
        __m128i v6 = _mm_or_si128( v5, v4 );

        int w = _mm_movemask_epi8( v6 );

        if( w != 0 ) break;

        s += 16;
        n -= 16;
    }

    return s - s0;
};

#else

inline
unsigned long long
count_unescaped(
    char const*,
    unsigned long long) noexcept
{
    return 0;
}

#endif

} // detail
} // json
} // boost

#endif
