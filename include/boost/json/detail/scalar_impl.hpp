//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_SCALAR_IMPL_HPP
#define BOOST_JSON_DETAIL_SCALAR_IMPL_HPP

#include <boost/json/kind.hpp>
#include <boost/json/storage_ptr.hpp>
#include <cstdint>

namespace boost {
namespace json {
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
        : sp(move(sp_))
        , k(kind::int64)
        , i(0)
    {
    }

    int64_k(
        std::int64_t i_,
        storage_ptr sp_) noexcept
        : sp(move(sp_))
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
        : sp(move(sp_))
        , k(kind::uint64)
        , u(0)
    {
    }

    uint64_k(
        std::uint64_t u_,
        storage_ptr sp_) noexcept
        : sp(move(sp_))
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
        : sp(move(sp_))
        , k(kind::double_)
        , d(0)
    {
    }

    double_k(
        double d_,
        storage_ptr sp_) noexcept
        : sp(move(sp_))
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
        : sp(move(sp_))
        , k(kind::bool_)
        , b(false)
    {
    }

    bool_k(
        bool b_,
        storage_ptr sp_) noexcept
        : sp(move(sp_))
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
        : sp(move(sp_))
        , k(kind::null)
    {
    }
};

} // detail
} // json
} // boost

#endif
