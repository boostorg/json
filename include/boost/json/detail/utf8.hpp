//
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_DETAIL_UTF8_HPP
#define BOOST_JSON_DETAIL_UTF8_HPP

#include <cstddef>
#include <cstring>
#include <cstdint>

namespace boost {
namespace json {
namespace detail {

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
    return first[static_cast<unsigned char>(c)];
}

inline
bool
is_valid_utf8(const char* p, uint16_t first)
{
    // KRYSTIAN TODO: account for big endian
    // also, make a convenience function for 
    // N-byte loads
    uint32_t v;
    switch(first >> 8)
    {
    default:
        return false;
    // 2 bytes, second byte [80, BF]
    case 1:
        std::memcpy(&v, p, 2);
        return (v & 0xC000) == 0x8000;
    // 3 bytes, second byte [A0, BF]
    case 2:
        std::memcpy(&v, p, 3);
        return (v & 0xC0E000) == 0x80A000;
    // 3 bytes, second byte [80, BF]
    case 3:
        std::memcpy(&v, p, 3);
        return (v & 0xC0C000) == 0x808000;
    // 3 bytes, second byte [80, 9F]
    case 4:
        std::memcpy(&v, p, 3);
        return (v & 0xC0E000) == 0x808000;
    // 4 bytes, second byte [90, BF]
    case 5:
        std::memcpy(&v, p, 4);
        return (v & 0xC0C0FF00) + 0x7F7F7000 <= 0x2F00;
    // 4 bytes, second byte [80, BF]
    case 6:
        std::memcpy(&v, p, 4);
        return (v & 0xC0C0C000) == 0x80808000;
    // 4 bytes, second byte [80, 8F]
    case 7:
        std::memcpy(&v, p, 4);
        return (v & 0xC0C0F000) == 0x80808000;
    }
}

struct utf8_sequence
{
private:
    char seq_[4];
    uint16_t first_;
    uint8_t size_;
public:
    void
    save(
        const char* p,
        std::size_t remain) noexcept
    {
        first_ = classify_utf8(*p & 0x7F);
        size_ = remain >= length() ? 
            length() : static_cast<uint8_t>(remain);
        std::memcpy(seq_, p, size_);
    }

    uint8_t 
    length() const noexcept
    {
        return first_ & 0xFF;
    }

    bool complete() const noexcept
    {
        return size_ >= length();
    }

    // return true if complete
    bool
    append(
        const char* p, 
        std::size_t remain) noexcept
    {
        if(! needed())
            return true;
        if(remain >= needed())
        {
            std::memcpy(seq_ + size_, p, needed());
            size_ = length();
            return true;
        }
        std::memcpy(seq_ + size_, p, remain);
        size_ += static_cast<uint8_t>(remain);
        return false;
    }

    const char*
    sequence() const noexcept
    {
        return seq_;
    }

    uint8_t needed() const noexcept
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

}
}
}

#endif