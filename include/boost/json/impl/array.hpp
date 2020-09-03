//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_IMPL_ARRAY_HPP
#define BOOST_JSON_IMPL_ARRAY_HPP

#include <boost/json/value.hpp>
#include <boost/json/detail/except.hpp>
#include <algorithm>
#include <stdexcept>
#include <type_traits>

BOOST_JSON_NS_BEGIN

class array::undo_construct
{
    array& self_;

public:
    bool commit = false;

    BOOST_JSON_DECL
    ~undo_construct();

    explicit
    undo_construct(
        array& self) noexcept
        : self_(self)
    {
    }
};

//----------------------------------------------------------

class array::undo_insert
{
    array& self_;
    std::size_t const n_;

public:
    value* it;
    std::size_t const pos;
    bool commit = false;

    BOOST_JSON_DECL
    undo_insert(
        value const* pos_,
        std::size_t n,
        array& self);

    BOOST_JSON_DECL
    ~undo_insert();

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

value&
array::
at(std::size_t pos)
{
    if(pos >= impl_.size())
        detail::throw_out_of_range(
            BOOST_CURRENT_LOCATION);
    return impl_.data()[pos];
}

value const&
array::
at(std::size_t pos) const
{
    if(pos >= impl_.size())
        detail::throw_out_of_range(
            BOOST_CURRENT_LOCATION);
    return impl_.data()[pos];
}

value&
array::
operator[](std::size_t pos) noexcept
{
    return impl_.data()[pos];
}

value const&
array::
operator[](std::size_t pos) const noexcept
{
    return impl_.data()[pos];
}

value&
array::
front() noexcept
{
    return *impl_.data();
}

value const&
array::
front() const noexcept
{
    return *impl_.data();
}

value&
array::
back() noexcept
{
    return impl_.data()[impl_.size() - 1];
}

value const&
array::
back() const noexcept
{
    return impl_.data()[impl_.size() - 1];
}


value*
array::
data() noexcept
{
    return impl_.data();
}

value const*
array::
contains(std::size_t pos) const noexcept
{
    if( pos < size() )
        return impl_.data() + pos;
    return nullptr;
}

value*
array::
contains(std::size_t pos) noexcept
{
    if( pos < size() )
        return impl_.data() + pos;
    return nullptr;
}

value const*
array::
data() const noexcept
{
    return impl_.data();
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
    return impl_.data();
}

auto
array::
begin() const noexcept ->
    const_iterator
{
    return impl_.data();
}

auto
array::
cbegin() const noexcept ->
    const_iterator
{
    return impl_.data();
}

auto
array::
end() noexcept ->
    iterator
{
    return impl_.data() + impl_.size();
}

auto
array::
end() const noexcept ->
    const_iterator
{
    return impl_.data() + impl_.size();
}

auto
array::
cend() const noexcept ->
    const_iterator
{
    return impl_.data() + impl_.size();
}

auto
array::
rbegin() noexcept ->
    reverse_iterator
{
    return reverse_iterator(
        impl_.data() + impl_.size());
}

auto
array::
rbegin() const noexcept ->
    const_reverse_iterator
{
    return const_reverse_iterator(
        impl_.data() + impl_.size());
}

auto
array::
crbegin() const noexcept ->
    const_reverse_iterator
{
    return const_reverse_iterator(
        impl_.data() + impl_.size());
}

auto
array::
rend() noexcept ->
    reverse_iterator
{
    return reverse_iterator(
        impl_.data());
}

auto
array::
rend() const noexcept ->
    const_reverse_iterator
{
    return const_reverse_iterator(
        impl_.data());
}

auto
array::
crend() const noexcept ->
    const_reverse_iterator
{
    return const_reverse_iterator(
        impl_.data());
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
    u.commit = true;
    return impl_.data() + u.pos;
}

template<class Arg>
value&
array::
emplace_back(Arg&& arg)
{
    reserve(impl_.size() + 1);
    auto& v = *::new(
        impl_.data() + impl_.size()) value(
            std::forward<Arg>(arg), sp_);
    impl_.size(impl_.size() + 1);
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
    undo_construct u(*this);
    while(first != last)
    {
        if(impl_.size() >= impl_.capacity())
            reserve(impl_.size() + 1);
        ::new(impl_.data() + impl_.size()) value(
            *first++, sp_);
        impl_.size(impl_.size() + 1);
    }
    u.commit = true;
}

template<class InputIt>
array::
array(
    InputIt first, InputIt last,
    storage_ptr sp,
    std::forward_iterator_tag)
    : sp_(std::move(sp))
{
    undo_construct u(*this);
    auto const n =
        static_cast<std::size_t>(
            std::distance(first, last));
    if(n > max_size())
        detail::throw_length_error(
            "array too large",
            BOOST_CURRENT_LOCATION);
    reserve(static_cast<std::size_t>(n));
    while(impl_.size() < n)
    {
        ::new(
            impl_.data() +
            impl_.size()) value(
                *first++, sp_);
        impl_.size(impl_.size() + 1);
    }
    u.commit = true;
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
        return impl_.data() +
            impl_.index_of(pos);
    array tmp(first, last, sp_);
    undo_insert u(
        pos, tmp.impl_.size(), *this);
    relocate(u.it,
        tmp.impl_.data(), tmp.impl_.size());
    // don't destroy values in tmp
    tmp.impl_.size(0);
    u.commit = true;
    return impl_.data() + u.pos;
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
    auto const n =
        static_cast<std::size_t>(
            std::distance(first, last));
    if(n > max_size())
        detail::throw_length_error(
            "array too large",
            BOOST_CURRENT_LOCATION);
    undo_insert u(pos, static_cast<
        std::size_t>(n), *this);
    while(first != last)
        u.emplace(*first++);
    u.commit = true;
    return data() + u.pos;
}

BOOST_JSON_NS_END

#endif
