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

class const_stream
{
    friend class local_const_stream;

    char const* p_;
    char const* end_;

public:
    using char_type = char;

    const_stream(
        const_stream const&) = default;

    const_stream(
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
    operator*() const noexcept
    {
        BOOST_ASSERT(p_ < end_);
        return *p_;
    }

    // unchecked
    const_stream&
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

class local_const_stream
    : public const_stream
{
    const_stream& src_;

public:
    explicit
    local_const_stream(
        const_stream& src)
        : const_stream(src)
        , src_(src)
    {
    }

    ~local_const_stream()
    {
        src_.p_ = p_;
    }

    void
    clip(std::size_t n)
    {
        if(static_cast<std::size_t>(
            src_.end_ - p_) > n)
            end_ = p_ + n;
        else
            end_ = src_.end_;
    }
};

//--------------------------------------

} // detail
} // json
} // boost

#endif
