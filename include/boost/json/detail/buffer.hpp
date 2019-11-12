//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_BUFFER_HPP
#define BOOST_JSON_DETAIL_BUFFER_HPP

#include <boost/json/detail/config.hpp>
#include <cstring>

namespace boost {
namespace json {
namespace detail {

// A simple string-like temporary static buffer
template<unsigned long N>
class buffer
{
    char buf_[N];
    unsigned int size_ = 0;

public:
    using size_type = unsigned int;

    buffer() = default;

    string_view
    get() const noexcept
    {
        return {buf_, size_};
    }

    operator string_view() const noexcept
    {
        return get();
    }

    char const*
    data() const noexcept
    {
        return buf_;
    }

    size_type
    size() const noexcept
    {
        return size_;
    }

    size_type
    max_size() const noexcept
    {
        return N;
    }

    void
    clear() noexcept
    {
        size_ = 0;
    }

    void
    push_back(char ch) noexcept
    {
        buf_[size_++] = ch;
    }

    // returns true if cp is a valid utf-32 code point
    static
    bool
    is_valid(unsigned long cp) noexcept
    {
        return cp <= 0x0010ffffu &&
            (cp < 0xd800u || cp > 0xdfffu);
    }

    // append valid 32-bit code point as utf8
    void
    append_utf8(
        unsigned long cp) noexcept
    {
        auto dest = buf_ + size_;
        if(cp < 0x80)
        {
            dest[0] = static_cast<char>(cp);
            size_ += 1;
            return;
        }

        if(cp < 0x800)
        {
            dest[0] = static_cast<char>( (cp >> 6)          | 0xc0);
            dest[1] = static_cast<char>( (cp & 0x3f)        | 0x80);
            size_ += 2;
            return;
        }

        if(cp < 0x10000)
        {
            dest[0] = static_cast<char>( (cp >> 12)         | 0xe0);
            dest[1] = static_cast<char>(((cp >> 6) & 0x3f)  | 0x80);
            dest[2] = static_cast<char>( (cp       & 0x3f)  | 0x80);
            size_ += 3;
            return;
        }

        {
            dest[0] = static_cast<char>( (cp >> 18)         | 0xf0);
            dest[1] = static_cast<char>(((cp >> 12) & 0x3f) | 0x80);
            dest[2] = static_cast<char>(((cp >> 6)  & 0x3f) | 0x80);
            dest[3] = static_cast<char>( (cp        & 0x3f) | 0x80);
            size_ += 4;
        }
    }
};

} // detail
} // json
} // boost

#endif
