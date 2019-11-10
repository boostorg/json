//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_STATIC_STACK_HPP
#define BOOST_JSON_DETAIL_STATIC_STACK_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/detail/assert.hpp>
#include <new>
#include <utility>

namespace boost {
namespace json {
namespace detail {

// NOTE T must be trivial
template<class T, std::size_t N>
class static_stack
{
    union
    {
        T vec_[N];
        int unused;
    };

    T* top_ = vec_ - 1;

public:
    using value_type = T;

    ~static_stack() = default;

    static_stack() noexcept
    {
    }

    std::size_t
    size() const noexcept
    {
        return top_ + 1 - vec_;
    }

    T&
    front() noexcept
    {
        return *top_;
    }

    T const&
    front() const noexcept
    {
        return *top_;
    }

    T&
    operator*() noexcept
    {
        return *top_;
    }

    T const&
    operator*() const noexcept
    {
        return *top_;
    }

    // capacity

    bool
    empty() const noexcept
    {
        return top_ < vec_;
    }

    void
    clear() noexcept
    {
        top_ = vec_ - 1;
    }

    void
    push(T const& t)
    {
        BOOST_JSON_ASSERT(
            top_ < vec_ + N);
        *++top_ = t;
    }

    template<class... Args>
    void
    emplace_front(Args&&... args)
    {
        BOOST_JSON_ASSERT(
            top_ < vec_ + N);
        ::new(++top_) T(
            std::forward<Args>(args)...);
    }

    void
    pop()
    {
        --top_;
    }
};

} // detail
} // json
} // boost

#endif
