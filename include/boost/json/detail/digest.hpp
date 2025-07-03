//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_DIGEST_HPP
#define BOOST_JSON_DETAIL_DIGEST_HPP

#include <algorithm>
#include <array>
#include <cstring>

namespace boost {
namespace json {
namespace detail {

// Calculate salted digest of string
template<class ForwardIterator>
std::size_t
digest(
    ForwardIterator b,
    ForwardIterator e,
    std::size_t salt) noexcept
{
#if BOOST_JSON_ARCH == 64
    using hash_t = std::uint64_t;
    hash_t const prime = 0x100000001B3ULL;
    hash_t hash  = 0xcbf29ce484222325ULL;
#else
    using hash_t = std::uint32_t;
    hash_t const prime = 0x01000193UL;
    hash_t hash  = 0x811C9DC5UL;
#endif
    hash += salt;

    constexpr std::size_t step = sizeof(hash_t);
    std::size_t n = std::distance(b, e);

    std::array<unsigned char, step> temp;
    hash_t batch;
    while( n >= step )
    {
        std::copy_n(b, step, temp.data());

        std::memcpy(&batch, temp.data(), step);
        hash = (batch ^ hash) * prime;

        std::advance(b, step);
        n -= step;
    }

    temp.fill(0);
    std::copy_n(b, n, temp.data());

    std::memcpy(&batch, temp.data(), step);
    hash = (batch ^ hash) * prime;

    return hash;
}

// Calculate salted digest of string
template<>
inline
std::size_t
digest<char const*>(
    char const* b,
    char const* e,
    std::size_t salt) noexcept
{
#if BOOST_JSON_ARCH == 64
    using hash_t = std::uint64_t;
    hash_t const prime = 0x100000001B3ULL;
    hash_t hash  = 0xcbf29ce484222325ULL;
#else
    using hash_t = std::uint32_t;
    hash_t const prime = 0x01000193UL;
    hash_t hash  = 0x811C9DC5UL;
#endif
    hash += salt;

    constexpr std::size_t step = sizeof(hash_t);

    hash_t batch;
    for( ; (e - b) >= static_cast<int>(step); b += step )
    {
        std::memcpy(&batch, b, step);
        hash = (batch ^ hash) * prime;
    }

    batch = 0;
    std::memcpy(&batch, b, e - b);
    hash = (batch ^ hash) * prime;

    return hash;
}


} // detail
} // namespace json
} // namespace boost

#endif
