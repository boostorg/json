//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_DETAIL_VALUE_HPP
#define BOOST_JSON_DETAIL_VALUE_HPP

#include <boost/json/kind.hpp>
#include <boost/json/storage_ptr.hpp>
#include <new>
#include <utility>

BOOST_JSON_NS_BEGIN
namespace detail {

struct int64_k
{
    storage_ptr sp; // must come first
    kind k;         // must come second
    std::int64_t i;

    int64_k() noexcept
        : k(kind::int64)
        , i(0)
    {
    }

    explicit
    int64_k(
        storage_ptr sp_) noexcept
        : sp(std::move(sp_))
        , k(kind::int64)
        , i(0)
    {
    }

    int64_k(
        std::int64_t i_,
        storage_ptr sp_) noexcept
        : sp(std::move(sp_))
        , k(kind::int64)
        , i(i_)
    {
    }
};

struct uint64_k
{
    storage_ptr sp; // must come first
    kind k;         // must come second
    std::uint64_t u;

    uint64_k() noexcept
        : k(kind::uint64)
        , u(0)
    {
    }

    explicit
    uint64_k(
        storage_ptr sp_) noexcept
        : sp(std::move(sp_))
        , k(kind::uint64)
        , u(0)
    {
    }

    uint64_k(
        std::uint64_t u_,
        storage_ptr sp_) noexcept
        : sp(std::move(sp_))
        , k(kind::uint64)
        , u(u_)
    {
    }
};

struct double_k
{
    storage_ptr sp; // must come first
    kind k;         // must come second
    double d;

    double_k() noexcept
        : k(kind::double_)
        , d(0)
    {
    }

    explicit
    double_k(
        storage_ptr sp_) noexcept
        : sp(std::move(sp_))
        , k(kind::double_)
        , d(0)
    {
    }

    double_k(
        double d_,
        storage_ptr sp_) noexcept
        : sp(std::move(sp_))
        , k(kind::double_)
        , d(d_)
    {
    }
};

struct bool_k
{
    storage_ptr sp; // must come first
    kind k;         // must come second
    bool b;

    bool_k() noexcept
        : k(kind::bool_)
        , b(false)
    {
    }

    explicit
    bool_k(
        storage_ptr sp_) noexcept
        : sp(std::move(sp_))
        , k(kind::bool_)
        , b(false)
    {
    }

    bool_k(
        bool b_,
        storage_ptr sp_) noexcept
        : sp(std::move(sp_))
        , k(kind::bool_)
        , b(b_)
    {
    }
};

struct null_k
{
    storage_ptr sp; // must come first
    kind k;         // must come second

    null_k() noexcept
        : k(kind::null)
    {
    }

    explicit
    null_k(
        storage_ptr sp_) noexcept
        : sp(std::move(sp_))
        , k(kind::null)
    {
    }
};

struct value_access
{
    template<class Value, class... Args>
    static
    Value&
    construct_value(Value* p, Args&&... args)
    {
        return *reinterpret_cast<
            Value*>(::new(p) Value(
            std::forward<Args>(args)...));
    }

    template<class KeyValuePair, class... Args>
    static
    KeyValuePair&
    construct_key_value_pair(
        KeyValuePair* p, Args&&... args)
    {
        return *reinterpret_cast<
            KeyValuePair*>(::new(p)
                KeyValuePair(
                    std::forward<Args>(args)...));
    }

    template<class Value>
    static
    char const*
    release_key(
        Value& jv,
        std::size_t& len) noexcept
    {
        BOOST_ASSERT(jv.is_string());
        return jv.str_.impl_.release_key(len);
    }

    using index_t = std::uint32_t;

    template<class KeyValuePair>
    static
    index_t&
    next(KeyValuePair& e) noexcept
    {
        return e.next_;
    }

    template<class KeyValuePair>
    static
    index_t const&
    next(KeyValuePair const& e) noexcept
    {
        return e.next_;
    }
};

} // detail
BOOST_JSON_NS_END

#endif
