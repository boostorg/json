//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_STREAM_HPP
#define BOOST_JSON_DETAIL_STREAM_HPP

namespace boost {
namespace json {
namespace detail {

class char_stream
{
    friend class local_char_stream;

    char const* p_;
    char const* end_;

public:
    using char_type = char;

    char_stream(char_stream const&) = default;

    char_stream(
        char const* data,
        std::size_t size)
        : p_(data)
        , end_(data + size)
    {
    }

    size_t
    used(char const* begin) const noexcept
    {
        return static_cast<
            size_t>(p_ - begin);
    }

    size_t
    remain() const noexcept
    {
        return end_ - p_;
    }

    char const*
    data() const noexcept
    {
        return p_;
    }

    operator bool() const noexcept
    {
        return p_ < end_;
    }

    // unchecked
    char
    operator[](int i) const noexcept
    {
        BOOST_ASSERT(i < remain());
        return p_[i];
    }

    // unchecked
    char
    operator*() const noexcept
    {
        BOOST_ASSERT(p_ < end_);
        return *p_;
    }

    // unchecked
    char_stream&
    operator++() noexcept
    {
        BOOST_ASSERT(p_ < end_);
        ++p_;
        return *this;
    }

    void
    skip(std::size_t n) noexcept
    {
        BOOST_ASSERT(n <= remain());
        p_ += n;
    }
};

class local_char_stream : public char_stream
{
    char_stream& src_;

public:
    explicit
    local_char_stream(
        char_stream& src)
        : char_stream(src)
        , src_(src)
    {
    }

    ~local_char_stream()
    {
        src_.p_ = p_;
    }

    void
    clip(std::size_t n)
    {
        if(remain() > n)
            end_ = p_ + n;
        else
            end_ = src_.end_;
    }
};

template<class Stream>
bool
consume(
    Stream& s,
    typename Stream::char_type ch)
{
    if(BOOST_JSON_LIKELY(
        s.peek() == ch))
    {
        s.get();
        return true;
    }
    return false;
}

} // detail
} // json
} // boost

#endif
