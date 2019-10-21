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
#include <algorithm>
#include <stdexcept>
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
    void
    destroy(
        storage_ptr const& sp) noexcept;

    BOOST_JSON_DECL
    static
    table*
    construct(
        size_type capacity,
        storage_ptr const& sp);

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
        value* src,
        size_type n) noexcept;
};

//------------------------------------------------------------------------------

struct array::undo_create
{
private:
    array& self_;
    table* saved_ = nullptr;
    bool commit_ = false;

public:
    BOOST_JSON_DECL
    ~undo_create();

    BOOST_JSON_DECL
    explicit
    undo_create(
        array& self) noexcept;

    BOOST_JSON_DECL
    undo_create(
        size_type n,
        array& self);

    void
    commit() noexcept
    {
        commit_ = true;
    }
};

//------------------------------------------------------------------------------

struct array::undo_insert
{
    value* it;
    array& self;
    size_type const pos;
    size_type const n;
    bool commit = false;

    BOOST_JSON_DECL
    undo_insert(
        value const* pos_,
        size_type n_,
        array& self_);

    BOOST_JSON_DECL
    ~undo_insert();

    template<class Arg>
    void
    emplace(Arg&& arg)
    {
        ::new(it) value(
            std::forward<Arg>(arg),
            self.sp_);
        ++it;
    }
};

//------------------------------------------------------------------------------
//
// Element access
//
//------------------------------------------------------------------------------

auto
array::
at(size_type pos) ->
    reference
{
    if(pos >= size())
        BOOST_JSON_THROW(std::out_of_range(
            "json::array index out of bounds"));
    return tab_->begin()[pos];
}

auto
array::
at(size_type pos) const ->
    const_reference
{
    if(pos >= size())
        BOOST_JSON_THROW(std::out_of_range(
            "json::array index out of bounds"));
    return tab_->begin()[pos];
}

auto
array::
operator[](size_type pos) ->
    reference
{
    return tab_->begin()[pos];
}

auto
array::
operator[](size_type pos) const ->
    const_reference
{
    return tab_->begin()[pos];
}

auto
array::
front() ->
    reference
{
    return tab_->begin()[0];
}

auto
array::
front() const ->
    const_reference
{
    return tab_->begin()[0];
}

auto
array::
back() ->
    reference
{
    return tab_->end()[-1];
}

auto
array::
back() const ->
    const_reference
{
    return tab_->end()[-1];
}

auto
array::
data() noexcept ->
    value*
{
    if(! tab_)
        return nullptr;
    return tab_->begin();
}

auto
array::
data() const noexcept ->
    value const*
{
    if(! tab_)
        return nullptr;
    return tab_->begin();
}

//------------------------------------------------------------------------------
//
// Iterators
//
//------------------------------------------------------------------------------

auto
array::
begin() noexcept ->
    iterator
{
    if(! tab_)
        return nullptr;
    return tab_->begin();
}

auto
array::
begin() const noexcept ->
    const_iterator
{
    if(! tab_)
        return nullptr;
    return tab_->begin();
}

auto
array::
cbegin() const noexcept ->
    const_iterator
{
    if(! tab_)
        return nullptr;
    return tab_->begin();
}

auto
array::
end() noexcept ->
    iterator
{
    if(! tab_)
        return nullptr;
    return tab_->end();
}

auto
array::
end() const noexcept ->
    const_iterator
{
    if(! tab_)
        return nullptr;
    return tab_->end();
}

auto
array::
cend() const noexcept ->
    const_iterator
{
    if(! tab_)
        return nullptr;
    return tab_->end();
}

auto
array::
rbegin() noexcept ->
    reverse_iterator
{
    if(! tab_)
        return reverse_iterator(nullptr);
    return reverse_iterator(tab_->end());
}

auto
array::
rbegin() const noexcept ->
    const_reverse_iterator
{
    if(! tab_)
        return const_reverse_iterator(nullptr);
    return const_reverse_iterator(tab_->end());
}

auto
array::
crbegin() const noexcept ->
    const_reverse_iterator
{
    if(! tab_)
        return const_reverse_iterator(nullptr);
    return const_reverse_iterator(tab_->end());
}

auto
array::
rend() noexcept ->
    reverse_iterator
{
    if(! tab_)
        return reverse_iterator(nullptr);
    return reverse_iterator(tab_->begin());
}

auto
array::
rend() const noexcept ->
    const_reverse_iterator
{
    if(! tab_)
        return const_reverse_iterator(nullptr);
    return const_reverse_iterator(tab_->begin());
}

auto
array::
crend() const noexcept ->
    const_reverse_iterator
{
    if(! tab_)
        return const_reverse_iterator(nullptr);
    return const_reverse_iterator(tab_->begin());
}

//------------------------------------------------------------------------------
//
// Capacity
//
//------------------------------------------------------------------------------

bool
array::
empty() const noexcept
{
    return ! tab_ || tab_->d.size == 0;
}

auto
array::
size() const noexcept ->
    size_type
{
    if(! tab_)
        return 0;
    return tab_->d.size;
}

auto
array::
max_size() const noexcept ->
    size_type
{
    return (std::numeric_limits<
        size_type>::max)() / sizeof(value);
}

auto
array::
capacity() const noexcept ->
    size_type
{
    if(! tab_)
        return 0;
    return tab_->d.capacity;
}

//------------------------------------------------------------------------------

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

template<class InputIt, class>
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
    undo_create u(*this);
    while(first != last)
    {
        if(size() >= capacity())
            reserve(size() + 1);
        ::new(tab_->end()) value(
            *first++, sp_);
        ++tab_->d.size;
    }
    u.commit();
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
    if(first == last)
        return begin() + (pos - begin());
    array tmp(first, last, sp_);
    undo_insert u(pos, tmp.size(), *this);
    table::relocate(
        u.it,
        tmp.tab_->begin(),
        tmp.size());
    tmp.tab_->destroy(sp_);
    tmp.tab_ = nullptr;
    u.commit = true;
    return begin() + u.pos;
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
    undo_insert u(pos, 1, *this);
    u.emplace(std::forward<Arg>(arg));
    u.commit = true;
    return begin() + u.pos;
}

} // json
} // boost

#endif
