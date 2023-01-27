//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_STACK_HPP
#define BOOST_JSON_DETAIL_STACK_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/static_assert.hpp>
#include <cstring>
#include <type_traits>

namespace boost {
namespace json {
namespace detail {

class stack
{
    struct non_trivial;

    storage_ptr sp_;
    std::size_t cap_ = 0;
    std::size_t size0_ = 0; // lo stack
    std::size_t size1_ = 0; // hi stack
    non_trivial* head_ = nullptr;
    unsigned char* base_ = nullptr;
    unsigned char* buf_ = nullptr;

public:
    BOOST_JSON_DECL
    ~stack();

    stack() = default;

    stack(
        storage_ptr sp,
        unsigned char* buf,
        std::size_t buf_size) noexcept;

    bool
    empty() const noexcept
    {
        return size0_ + size1_ == 0;
    }

    BOOST_JSON_DECL
    void
    clear() noexcept;

    void
    reserve(std::size_t n)
    {
        if(n > cap_)
            reserve_impl(n);
    }

    template<class T>
    void
    push(T const& t)
    {
        push(t, std::is_trivial<T>{});
    }

    template<class T>
    void
    push_unchecked(
        T const& t);

    template<class T>
    void
    peek(T& t);

    template<class T>
    void
    pop(T& t)
    {
        pop(t, std::is_trivial<T>{});
    }

private:
    template<class T> void push(
        T const& t, std::true_type);
    template<class T> void push(
        T const& t, std::false_type);
    template<class T> void pop(
        T& t, std::true_type);
    template<class T> void pop(
        T& t, std::false_type);

    BOOST_JSON_DECL
    void
    reserve_impl(
        std::size_t n);
};

} // detail
} // json
} // boost

#include <boost/json/detail/impl/stack.hpp>

#endif