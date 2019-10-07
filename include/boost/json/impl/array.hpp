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

struct array::undo_create
{
private:
    array& self_;
    table* saved_;
    bool commit_ = false;

public:
    undo_create(
        size_type n,
        array& self)
        : self_(self)
        , saved_(boost::exchange(
            self.tab_,
            table::create(n, self.sp_)))
    {
    }

    void
    commit()
    {
        commit_ = true;
    }

    ~undo_create()
    {
        if(! commit_)
        {
            table::destroy(
                self_.tab_, self_.sp_);
            self_.tab_ = saved_;
        }
        else if(saved_)
        {
            table::destroy(
                saved_, self_.sp_);
        }
    }
};

struct array::undo_insert
{
    value* it;
    array& self;
    size_type const pos;
    size_type const n;
    bool commit = false;

    inline
    undo_insert(
        value const* pos_,
        size_type n_,
        array& self_);

    inline
    ~undo_insert();

    template<class Arg>
    void
    emplace(Arg&& arg);
};

//------------------------------------------------------------------------------

template<class InputIt, class>
array::
array(
    InputIt first, InputIt last)
    : array(
        first, last,
        default_storage(),
        iter_cat<InputIt>{})
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
    storage_ptr sp)
    : array(
        first, last,
        std::move(sp),
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
    storage_ptr sp,
    std::input_iterator_tag)
    : sp_(std::move(sp))
{
}

template<class InputIt>
array::
array(
    InputIt first, InputIt last,
    storage_ptr sp,
    std::forward_iterator_tag)
    : sp_(std::move(sp))
{
    undo_create u(std::distance(
        first, last), *this);
    while(first != last)
    {
        ::new(tab_->end()) value(
            *first++, sp_);
        ++tab_->d.size;
    }
    u.commit();
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
    // TODO
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
    undo_insert u(
        pos, std::distance(
            first, last), *this);
    while(first != last)
        u.emplace(*first++);
    u.commit = true;
    return begin() + u.pos;
}

template<class Arg>
auto
array::
emplace_impl(
    const_iterator pos,
    Arg&& arg) ->
        iterator
{
    undo_insert u(
        pos, 1, *this);
    u.emplace(
        std::forward<Arg>(arg));
    u.commit = true;
    return begin() + u.pos;
}

} // json
} // boost

#endif
