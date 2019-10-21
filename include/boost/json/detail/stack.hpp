//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_STACK_HPP
#define BOOST_JSON_DETAIL_STACK_HPP

#include <boost/json/detail/assert.hpp>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

namespace boost {
namespace json {
namespace detail {

template<class T, std::size_t N>
class stack
{
    std::vector<T> v_; 
    std::size_t n_ = 0; // includes v_.size()
    typename std::aligned_storage<
        N * sizeof(T), alignof(T)>::type buf_;

    T*
    base() noexcept
    {
        return reinterpret_cast<
            T*>(&buf_);
    }

    T const*
    base() const noexcept
    {
        return reinterpret_cast<
            T const*>(&buf_);
    }

    void
    destroy()
    {
        for(auto p = base(),
            last = base() +
                (n_ < N ? n_ : N);
            p != last; ++p)
        {
            (*p).~T();
        }
    }

public:
    using value_type = T;

    stack() = default;

    ~stack()
    {
        destroy();
    }

    // element access

    T&
    operator[](std::size_t i) noexcept
    {
        BOOST_JSON_ASSERT(i < n_);
        if(v_.empty())
            return base()[n_ - (i + 1)];
        if(i < v_.size())
            return v_[v_.size() - (i + 1)];
        return base()[
            N - (v_.size() + i + 1)];
    }

    T const&
    operator[](std::size_t i) const noexcept
    {
        BOOST_JSON_ASSERT(i < n_);
        if(v_.empty())
            return base()[n_ - (i + 1)];
        if(i < v_.size())
            return v_[v_.size() - (i + 1)];
        return base()[
            N - (v_.size() + i + 1)];
    }

    T&
    front() noexcept
    {
        return (*this)[0];
    }

    T const&
    front() const noexcept
    {
        return (*this)[0];
    }

    // capacity

    bool
    empty() const noexcept
    {
        return n_ == 0;
    }

    std::size_t
    size() const noexcept
    {
        return n_;
    }

    std::size_t
    max_size() const noexcept
    {
        return v_.max_size();
    }

    void
    reserve(std::size_t n)
    {
        if(n <= N)
            return;
        v_.reserve(n - N);
    }

    std::size_t
    capacity() const noexcept
    {
        return N + v_.capacity();
    }

    // modifiers

    void
    clear() noexcept
    {
        v_.clear();
        destroy();
        n_ = 0;
    }

    void
    push(T const& t)
    {
        if(n_ < N)
            ::new(&base()[n_]) T(t);
        else
            v_.push_back(t);
        ++n_;
    }

    template<class... Args>
    void
    emplace_front(Args&&... args)
    {
        if(n_ < N)
            ::new(&base()[n_]) T(
                std::forward<Args>(args)...);
        else
            v_.emplace_back(
                std::forward<Args>(args)...);
        ++n_;
    }

    void
    pop()
    {
        BOOST_JSON_ASSERT(n_ > 0);
        if(! v_.empty())
            v_.pop_back();
        else
            base()[n_ - 1].~T();
        --n_;
    }
};

} // detail
} // json
} // boost

#endif
