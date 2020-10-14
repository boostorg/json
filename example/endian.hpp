//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_EXAMPLE_ENDIAN_HPP
#define BOOST_JSON_EXAMPLE_ENDIAN_HPP

#include <boost/json/detail/config.hpp>
#include <cstdint>

// We use our own endian routines, to support the standalone mode.

namespace detail {

inline
void
reverse(
    unsigned char*,
    std::integral_constant<
        std::size_t, 1>)
{
}

inline
void
reverse(
    unsigned char* p,
    std::integral_constant<
        std::size_t, 2>)
{
    unsigned char tmp[2];
    std::memcpy(p, tmp, sizeof(tmp));
    p[0] = tmp[1];
    p[1] = tmp[0];
}

inline
void
reverse(
    unsigned char* p,
    std::integral_constant<
        std::size_t, 4>)
{
    unsigned char tmp[4];
    std::memcpy(p, tmp, sizeof(tmp));
    p[0] = tmp[3];
    p[1] = tmp[2];
    p[2] = tmp[1];
    p[3] = tmp[0];
}

inline
void
reverse(
    unsigned char* p,
    std::integral_constant<
        std::size_t, 8>)
{
    unsigned char tmp[8];
    std::memcpy(p, tmp, sizeof(tmp));
    p[0] = tmp[7];
    p[1] = tmp[6];
    p[2] = tmp[5];
    p[3] = tmp[4];
    p[4] = tmp[3];
    p[5] = tmp[2];
    p[6] = tmp[1];
    p[7] = tmp[0];
}

} // detail

template<class T>
void
big_endian_store(
    unsigned char* p,
    T const& v) noexcept
{
    std::memcpy(p, &v, sizeof(v));
#ifndef BOOST_JSON_BIG_ENDIAN
    detail::reverse(p, std::integral_constant<
        std::size_t, sizeof(T)>{});
#endif
}

inline
std::uint16_t
load_big_u16(unsigned char const* p) noexcept
{
    std::uint16_t v;
    std::memcpy(&v, p, sizeof(v));
#ifndef BOOST_JSON_BIG_ENDIAN
    detail::reverse(
        reinterpret_cast<unsigned char*>(&v),
        std::integral_constant<
            std::size_t, sizeof(v)>{});
#endif
    return v;
}

inline
std::uint32_t
load_big_u32(unsigned char const* p) noexcept
{
    std::uint32_t v;
    std::memcpy(&v, p, sizeof(v));
#ifndef BOOST_JSON_BIG_ENDIAN
    detail::reverse(
        reinterpret_cast<unsigned char*>(&v),
        std::integral_constant<
            std::size_t, sizeof(v)>{});
#endif
    return v;
}

inline
std::uint64_t
load_big_u64(unsigned char const* p) noexcept
{
    std::uint64_t v;
    std::memcpy(&v, p, sizeof(v));
#ifndef BOOST_JSON_BIG_ENDIAN
    detail::reverse(
        reinterpret_cast<unsigned char*>(&v),
        std::integral_constant<
            std::size_t, sizeof(v)>{});
#endif
    return v;
}

inline
float
load_big_float(unsigned char const* p) noexcept
{
    float v;
    std::memcpy(&v, p, sizeof(v));
#ifndef BOOST_JSON_BIG_ENDIAN
    detail::reverse(
        reinterpret_cast<unsigned char*>(&v),
        std::integral_constant<
            std::size_t, sizeof(v)>{});
#endif
    return v;
}

inline
double
load_big_double(unsigned char const* p) noexcept
{
    double v;
    std::memcpy(&v, p, sizeof(v));
#ifndef BOOST_JSON_BIG_ENDIAN
    detail::reverse(
        reinterpret_cast<unsigned char*>(&v),
        std::integral_constant<
            std::size_t, sizeof(v)>{});
#endif
    return v;
}

#endif
