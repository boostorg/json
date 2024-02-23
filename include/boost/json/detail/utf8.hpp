//
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_UTF8_HPP
#define BOOST_JSON_DETAIL_UTF8_HPP

#include <boost/endian/conversion.hpp>
#include <boost/json/detail/config.hpp>

#include <cstddef>
#include <cstring>
#include <cstdint>

namespace boost {
namespace json {
namespace detail {

template<endian::order = endian::order::little>
constexpr
std::uint32_t
make_u32_impl(std::uint8_t b4, std::uint8_t b3, std::uint8_t b2, std::uint8_t b1)
{
    return (b4 << 24) | (b3 << 16) | (b2 << 8) | b1;
}

template<>
constexpr
std::uint32_t
make_u32_impl<endian::order::big>(
    std::uint8_t b4, std::uint8_t b3, std::uint8_t b2, std::uint8_t b1)
{
    return (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
}

constexpr
std::uint32_t
make_u32(std::uint8_t b4, std::uint8_t b3, std::uint8_t b2, std::uint8_t b1)
{
    return make_u32_impl<endian::order::native>(b4, b3, b2, b1);
}

template<endian::order = endian::order::little>
constexpr
bool
utf8_case5(std::uint32_t v)
{
    return ( ( ( v & make_u32(0xC0,0xC0,0xF0,0x00) )
        + make_u32(0x7F,0x7F,0x70,0x00) ) | make_u32(0x00,0x00,0x30,0x00) )
        == make_u32(0x00,0x00,0x30,0x00);
}

template<>
constexpr
bool
utf8_case5<endian::order::big>(std::uint32_t v)
{
    return ( ( ( v & make_u32(0xC0,0xC0,0xF0,0x00) )
        + make_u32(0x00,0x00,0x70,0xFF) ) | make_u32(0x00,0x00,0x30,0x00) )
        == make_u32(0x80,0x80,0x30,0x00);
}

template<int N>
std::uint32_t
load_little_endian(void const* p)
{
    std::uint32_t v = 0;
    std::memcpy(&v, p, N);
    endian::little_to_native_inplace(v);
    return v;
}

inline
uint16_t
classify_utf8(char c)
{
    // 0x000 = invalid
    // 0x102 = 2 bytes, second byte [80, BF]
    // 0x203 = 3 bytes, second byte [A0, BF]
    // 0x303 = 3 bytes, second byte [80, BF]
    // 0x403 = 3 bytes, second byte [80, 9F]
    // 0x504 = 4 bytes, second byte [90, BF]
    // 0x604 = 4 bytes, second byte [80, BF]
    // 0x704 = 4 bytes, second byte [80, 8F]
    static constexpr uint16_t first[128]
    {
       0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
       0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
       0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
       0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
       0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
       0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
       0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
       0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,

       0x000, 0x000, 0x102, 0x102, 0x102, 0x102, 0x102, 0x102,
       0x102, 0x102, 0x102, 0x102, 0x102, 0x102, 0x102, 0x102,
       0x102, 0x102, 0x102, 0x102, 0x102, 0x102, 0x102, 0x102,
       0x102, 0x102, 0x102, 0x102, 0x102, 0x102, 0x102, 0x102,
       0x203, 0x303, 0x303, 0x303, 0x303, 0x303, 0x303, 0x303,
       0x303, 0x303, 0x303, 0x303, 0x303, 0x403, 0x303, 0x303,
       0x504, 0x604, 0x604, 0x604, 0x704, 0x000, 0x000, 0x000,
       0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
    };
    return first[static_cast<unsigned char>(c & 0x7F)];
}

inline
bool
is_valid_utf8(const char* p, uint16_t first)
{
    std::uint32_t v;
    switch(first >> 8)
    {
    default:
        return false;

    // 2 bytes, second byte [80, BF]
    case 1:
        std::memcpy(&v, p, 2);
        return ( v & make_u32(0x00,0x00,0xC0,0x00) )
            == make_u32(0x00,0x00,0x80,0x00);

    // 3 bytes, second byte [A0, BF]
    case 2:
        std::memcpy(&v, p, 3);
        return ( v & make_u32(0x00,0xC0,0xE0,0x00) )
            == make_u32(0x00,0x80,0xA0,0x00);

    // 3 bytes, second byte [80, BF]
    case 3:
        std::memcpy(&v, p, 3);
        return ( v & make_u32(0x00,0xC0,0xC0,0x00) )
            == make_u32(0x00,0x80,0x80,0x00);

    // 3 bytes, second byte [80, 9F]
    case 4:
        std::memcpy(&v, p, 3);
        return ( v & make_u32(0x00,0xC0,0xE0,0x00) )
            == make_u32(0x00,0x80,0x80,0x00);

    // 4 bytes, second byte [90, BF]
    case 5:
        std::memcpy(&v, p, 4);
        return utf8_case5<endian::order::native>(v);

    // 4 bytes, second byte [80, BF]
    case 6:
        std::memcpy(&v, p, 4);
        return ( v & make_u32(0xC0,0xC0,0xC0,0x00) )
            == make_u32(0x80,0x80,0x80,0x00);

    // 4 bytes, second byte [80, 8F]
    case 7:
        std::memcpy(&v, p, 4);
        return ( v & make_u32(0xC0,0xC0,0xF0,0x00) )
            == make_u32(0x80,0x80,0x80,0x00);
    }
}

class utf8_sequence
{
    char seq_[4];
    uint16_t first_;
    uint8_t size_;

public:
    void
    save(
        const char* p,
        std::size_t remain) noexcept
    {
        first_ = classify_utf8(*p );
        if(remain >= length())
            size_ = length();
        else
            size_ = static_cast<uint8_t>(remain);
        std::memcpy(seq_, p, size_);
    }

    uint8_t
    length() const noexcept
    {
        return first_ & 0xFF;
    }

    bool
    complete() const noexcept
    {
        return size_ >= length();
    }

    // returns true if complete
    bool
    append(
        const char* p,
        std::size_t remain) noexcept
    {
        if(BOOST_JSON_UNLIKELY(needed() == 0))
            return true;
        if(BOOST_JSON_LIKELY(remain >= needed()))
        {
            std::memcpy(
                seq_ + size_, p, needed());
            size_ = length();
            return true;
        }
        if(BOOST_JSON_LIKELY(remain > 0))
        {
            std::memcpy(seq_ + size_, p, remain);
            size_ += static_cast<uint8_t>(remain);
        }
        return false;
    }

    const char*
    data() const noexcept
    {
        return seq_;
    }

    uint8_t
    needed() const noexcept
    {
        return length() - size_;
    }

    bool
    valid() const noexcept
    {
        BOOST_ASSERT(size_ >= length());
        return is_valid_utf8(seq_, first_);
    }
};

} // detail
} // namespace json
} // namespace boost

#endif
