//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_ARRAY_HPP
#define BOOST_JSON_IMPL_ARRAY_HPP

#include <boost/json/value.hpp>
#include <boost/core/exchange.hpp>
#include <boost/static_assert.hpp>
#include <algorithm>
#include <type_traits>

namespace boost {
namespace json {

//------------------------------------------------------------------------------

struct array::table
{
    struct data
    {
        size_type size;
        size_type capacity;
    };

    union
    {
        data d;
        value unused; // for alignment
    };

    ~table() = delete;

    table()
    {
    }

    value*
    begin() noexcept
    {
        return reinterpret_cast<
            value*>(this + 1);
    }

    value*
    end() noexcept
    {
        return reinterpret_cast<
            value*>(this + 1) + d.size;
    }

    BOOST_JSON_DECL
    static
    table*
    create(
        size_type capacity,
        storage_ptr const& sp);

    BOOST_JSON_DECL
    static
    void
    destroy(
        table* tab,
        storage_ptr const& sp) noexcept;

    BOOST_JSON_DECL
    static
    void
    destroy(
        value* first,
        value* last) noexcept;

    BOOST_JSON_DECL
    static
    void
    relocate(
        value* dest,
        value* first,
        value* last) noexcept;
};

//------------------------------------------------------------------------------

struct array::cleanup_assign
{
    array& self;
    table* tab;
    bool ok = false;

    BOOST_JSON_DECL
    explicit
    cleanup_assign(
        array& self);

    BOOST_JSON_DECL
    ~cleanup_assign();
};

//------------------------------------------------------------------------------

struct array::cleanup_insert
{
    array& self;
    size_type pos;
    size_type n;
    size_type valid = 0;
    bool ok = false;

    BOOST_JSON_DECL
    cleanup_insert(
        size_type pos_,
        size_type n_,
        array& self_);

    BOOST_JSON_DECL
    ~cleanup_insert();
};

//------------------------------------------------------------------------------

template<class InputIt, class>
array::
array(
    InputIt first, InputIt last)
    : array(
        first, last,
        default_storage())
{
    static_assert(
        std::is_constructible<value,
            decltype(*first)>::value,
        "json::value is not constructible from the iterator's value type");
}

template<class InputIt, class>
array::
array(
    InputIt first, InputIt last,
    storage_ptr store)
    : array(
        first, last,
        std::move(store),
        iter_cat<InputIt>{})
{
    static_assert(
        std::is_constructible<value,
            decltype(*first)>::value,
        "json::value is not constructible from the iterator's value type");
}

template<class InputIt>
auto
array::
insert(
    const_iterator pos,
    InputIt first, InputIt last) ->
        iterator
{
    static_assert(
        std::is_constructible<value,
            decltype(*first)>::value,
        "json::value is not constructible from the iterator's value type");
    return insert(pos, first, last,
        iter_cat<InputIt>{});
}

template<class Arg>
auto
array::
emplace(
    const_iterator pos,
    Arg&& arg) ->
        iterator
{
    return emplace_impl(
        pos, std::forward<Arg>(arg));
}

template<class Arg>
auto
array::
emplace_back(Arg&& arg) ->
    reference
{
    return *emplace_impl(
        end(), std::forward<Arg>(arg));
}

//------------------------------------------------------------------------------

template<class InputIt>
array::
array(
    InputIt first, InputIt last,
    storage_ptr store,
    std::input_iterator_tag)
    : sp_(std::move(store))
{
    while(first != last)
        emplace_impl(end(), *first++);
}

template<class InputIt>
array::
array(
    InputIt first, InputIt last,
    storage_ptr store,
    std::forward_iterator_tag)
    : sp_(std::move(store))
{
    reserve(std::distance(first, last));
    while(first != last)
        emplace_impl(end(), *first++);
}
template<class InputIt>
auto
array::
insert(
    const_iterator pos,
    InputIt first, InputIt last,
    std::input_iterator_tag) ->
        iterator
{
    auto d = pos - begin();
    while(first != last)
        pos = insert(pos, *first++) + 1;
    return begin() + d;
}

template<class InputIt>
auto
array::
insert(
    const_iterator pos,
    InputIt first, InputIt last,
    std::forward_iterator_tag) ->
        iterator
{
    auto count = std::distance(first, last);
    auto d = pos - begin();
    reserve(size() + count);
    cleanup_insert c(d, count, *this);
    while(count--)
    {
        ::new(&begin()[d++]) value(
            *first++, sp_);
        ++c.valid;
    }
    c.ok = true;
    return begin() + c.pos;
}

template<class Arg>
auto
array::
emplace_impl(
    const_iterator pos,
    Arg&& arg) ->
        iterator
{
    auto const d = pos - begin();
    reserve(size() + 1);
    cleanup_insert c(d, 1, *this);
    ::new(&tab_->begin()[d]) value(
        std::forward<Arg>(arg), sp_);
    c.ok = true;
    return begin() + d;
}

} // json
} // boost

#endif
