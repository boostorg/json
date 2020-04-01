//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_RAW_STACK_HPP
#define BOOST_JSON_DETAIL_RAW_STACK_HPP

#include <boost/json/config.hpp>
#include <boost/json/storage_ptr.hpp>
#include <cstdlib>
#include <utility>

namespace boost {
namespace json {
namespace detail {

class raw_stack
{
    storage_ptr sp_;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
    char* base_ = nullptr;

public:
    explicit
    raw_stack(
        storage_ptr sp = {})
        : sp_(std::move(sp))
    {
    }

    ~raw_stack()
    {
        if(base_)
            sp_->deallocate(
                base_, size_);
    }

    bool
    empty() const noexcept
    {
        return size_ == 0;
    }

    std::size_t
    top() const noexcept
    {
        return size_;
    }

    BOOST_JSON_DECL
    void
    reserve(std::size_t bytes);

    void
    clear() noexcept
    {
        size_ = 0;
    }

    char*
    push(std::size_t n)
    {
        prepare(n);
        auto p = base_ + size_;
        size_ += n;
        return p;
    }

    char*
    behind(std::size_t n) noexcept
    {
        return base_ + size_ - n;
    }

    char*
    pop(std::size_t n) noexcept
    {
        BOOST_ASSERT(
            n <= size_);
        size_ -= n;
        return base_ + size_;
    }

    void
    prepare(std::size_t n)
    {
        auto const avail =
            capacity_ - size_;
        if(n > avail)
            grow(n - avail);
    }

    void
    add(std::size_t n)
    {
        prepare(n);
        size_ += n;
    }

    void
    add_unchecked(std::size_t n)
    {
        BOOST_ASSERT(n <=
            capacity_ - size_);
        size_ += n;
    }

    void
    subtract(std::size_t n)
    {
        BOOST_ASSERT(
            n <= size_);
        size_ -= n;
    }

    template<class T>
    friend
    char
    align_to(raw_stack& rs) noexcept;

private:
    static
    constexpr
    std::size_t
    min_capacity_ = 1024;

    static
    constexpr
    std::size_t
    max_size() noexcept
    {
        return BOOST_JSON_MAX_STACK_SIZE;
    }

    BOOST_JSON_DECL
    void
    grow(std::size_t n);
};

template<class T>
char
align_to(raw_stack& rs) noexcept
{
    auto const align =
        static_cast<char>(
        alignof(T) * ((rs.size_ +
        alignof(T) - 1) / alignof(T)) -
        rs.size_);
    rs.size_ += align;
    return align;
}

} // detail
} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/detail/impl/raw_stack.ipp>
#endif

#endif
