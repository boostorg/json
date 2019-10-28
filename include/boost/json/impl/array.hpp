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
#include <algorithm>
#include <stdexcept>
#include <type_traits>

namespace boost {
namespace json {

auto
array::
impl_type::
index_of(value const* pos) const noexcept ->
    size_type
{
    return static_cast<
        size_type>(pos - vec);
}

//----------------------------------------------------------

class array::undo_create
{
    array& self_;
    bool commit_ = false;

public:
    BOOST_JSON_DECL
    ~undo_create();

    explicit
    undo_create(
        array& self) noexcept
        : self_(self)
    {
    }

    void
    commit() noexcept
    {
        commit_ = true;
    }
};

//----------------------------------------------------------

class array::undo_assign
{
    array& self_;
    impl_type impl_;
    bool commit_ = false;

public:
    BOOST_JSON_DECL
    ~undo_assign();

    explicit
    BOOST_JSON_DECL
    undo_assign(array& self);

    void
    commit() noexcept
    {
        commit_ = true;
    }
};

//----------------------------------------------------------

class array::undo_insert
{
    array& self_;
    size_type const n_;
    bool commit_ = false;

public:
    value* it;
    size_type const pos;

    BOOST_JSON_DECL
    undo_insert(
        value const* pos_,
        std::size_t n,
        array& self);

    BOOST_JSON_DECL
    ~undo_insert();

    void
    commit() noexcept
    {
        commit_ = true;
    }

    template<class Arg>
    void
    emplace(Arg&& arg)
    {
        ::new(it) value(
            std::forward<Arg>(arg),
            self_.sp_);
        ++it;
    }
};

//----------------------------------------------------------
//
// Element access
//
//----------------------------------------------------------

auto
array::
at(size_type pos) ->
    reference
{
    if(pos >= impl_.size)
        BOOST_JSON_THROW(
            std::out_of_range(
                "bad array index"));
    return impl_.vec[pos];
}

auto
array::
at(size_type pos) const ->
    const_reference
{
    if(pos >= impl_.size)
        BOOST_JSON_THROW(
            std::out_of_range(
                "bad array index"));
    return impl_.vec[pos];
}

auto
array::
operator[](size_type pos) noexcept ->
    reference
{
    return impl_.vec[pos];
}

auto
array::
operator[](size_type pos) const noexcept ->
const_reference
{
    return impl_.vec[pos];
}

auto
array::
front() noexcept ->
    reference
{
    return *impl_.vec;
}

auto
array::
front() const noexcept ->
    const_reference
{
    return *impl_.vec;
}

auto
array::
back() noexcept ->
    reference
{
    return impl_.vec[impl_.size - 1];
}

auto
array::
back() const noexcept ->
    const_reference
{
    return impl_.vec[impl_.size - 1];
}

auto
array::
data() noexcept ->
    value*
{
    return impl_.vec;
}

auto
array::
data() const noexcept ->
    value const*
{
    return impl_.vec;
}

//----------------------------------------------------------
//
// Iterators
//
//----------------------------------------------------------

auto
array::
begin() noexcept ->
    iterator
{
    return impl_.vec;
}

auto
array::
begin() const noexcept ->
    const_iterator
{
    return impl_.vec;
}

auto
array::
cbegin() const noexcept ->
    const_iterator
{
    return impl_.vec;
}

auto
array::
end() noexcept ->
    iterator
{
    return impl_.vec + impl_.size;
}

auto
array::
end() const noexcept ->
    const_iterator
{
    return impl_.vec + impl_.size;
}

auto
array::
cend() const noexcept ->
    const_iterator
{
    return impl_.vec + impl_.size;
}

auto
array::
rbegin() noexcept ->
    reverse_iterator
{
    return reverse_iterator(
        impl_.vec + impl_.size);
}

auto
array::
rbegin() const noexcept ->
    const_reverse_iterator
{
    return const_reverse_iterator(
        impl_.vec + impl_.size);
}

auto
array::
crbegin() const noexcept ->
    const_reverse_iterator
{
    return const_reverse_iterator(
        impl_.vec + impl_.size);
}

auto
array::
rend() noexcept ->
    reverse_iterator
{
    return reverse_iterator(
        impl_.vec);
}

auto
array::
rend() const noexcept ->
    const_reverse_iterator
{
    return const_reverse_iterator(
        impl_.vec);
}

auto
array::
crend() const noexcept ->
    const_reverse_iterator
{
    return const_reverse_iterator(
        impl_.vec);
}

//----------------------------------------------------------
//
// function templates
//
//----------------------------------------------------------

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
    undo_insert u(pos, 1, *this);
    u.emplace(std::forward<Arg>(arg));
    u.commit();
    return impl_.vec + u.pos;
}

template<class Arg>
auto
array::
emplace_back(Arg&& arg) ->
    reference
{
    reserve(impl_.size + 1);
    auto& v = *::new(
        impl_.vec + impl_.size) value(
            std::forward<Arg>(arg), sp_);
    ++impl_.size;
    return v;
}

//----------------------------------------------------------
//
// implementaiton
//
//----------------------------------------------------------

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
        if(impl_.size >= impl_.capacity)
            reserve(impl_.size + 1);
        ::new(impl_.vec + impl_.size) value(
            *first++, sp_);
        ++impl_.size;
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
    undo_create u(*this);
    auto const n = static_cast<std::size_t>(
        std::distance(first, last));
    if(n > max_size())
        BOOST_JSON_THROW(
            std::length_error(
                "n > max_size"));
    reserve(static_cast<size_type>(n));
    while(impl_.size < n)
    {
        ::new(
            impl_.vec +
            impl_.size) value(
                *first++, sp_);
        ++impl_.size;
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
        return impl_.vec +
            impl_.index_of(pos);
    array tmp(first, last, sp_);
    undo_insert u(
        pos, tmp.impl_.size, *this);
    relocate(u.it,
        tmp.impl_.vec, tmp.impl_.size);
    tmp.impl_.size = 0;
    tmp.impl_.destroy(sp_);
    u.commit();
    return impl_.vec + u.pos;
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
    auto const n = static_cast<std::size_t>(
        std::distance(first, last));
    if(n > max_size())
        BOOST_JSON_THROW(
            std::length_error(
                "n > max_size"));
    undo_insert u(pos, static_cast<
        size_type>(n), *this);
    while(first != last)
        u.emplace(*first++);
    u.commit();
    return begin() + u.pos;
}

} // json
} // boost

#endif
