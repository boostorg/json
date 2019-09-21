//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_ARRAY_IPP
#define BOOST_JSON_IMPL_ARRAY_IPP

#include <boost/json/array.hpp>
#include <boost/core/exchange.hpp>
#include <boost/assert.hpp>
#include <algorithm>
#include <cstdlib>
#include <limits>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace boost {
namespace json {

//------------------------------------------------------------------------------

array::
table::
~table()
{
    while(size > 0)
        begin()[--size].~value();
}

auto
array::
table::
create(
    size_type capacity,
    storage_ptr const& sp) ->
        table*
{
    return ::new(sp->allocate(
        sizeof(table) +
            capacity * sizeof(value_type),
        sizeof(value))) table(capacity);
}

void
array::
table::
destroy(
    table* tab,
    storage_ptr const& sp)
{
    auto const capacity = tab->capacity();
    tab->~table();
    sp->deallocate(tab,
        sizeof(table) +
            capacity * sizeof(value_type),
        sizeof(value));
}

//------------------------------------------------------------------------------

array::
cleanup_assign::
cleanup_assign(
    array& self_)
    : self(self_)
    , tab(boost::exchange(
        self_.tab_, nullptr))
{
}

array::
cleanup_assign::
~cleanup_assign()
{
    if(ok)
    {
        if(tab)
            table::destroy(tab, self.sp_);
    }
    else
    {
        if(self.tab_)
            table::destroy(
                self.tab_, self.sp_);
        self.tab_ = tab;
    }
}

//------------------------------------------------------------------------------

array::
cleanup_insert::
cleanup_insert(
    size_type pos_,
    size_type n_,
    array& self_)
    : self(self_)
    , pos(pos_)
    , n(n_)
{
    self.move(
        self.data() + pos + n,
        self.data() + pos,
        self.size() - pos);
}

array::
cleanup_insert::
~cleanup_insert()
{
    if(ok)
    {
        self.tab_->size += n;
    }
    else
    {
        for(size_type i = n;
            valid--; ++i)
            self[i].~value();

        self.move(
            self.data() + pos,
            self.data() + pos + n,
            self.size() - pos);
    }
}

//------------------------------------------------------------------------------
//
// Special Members
//
//------------------------------------------------------------------------------

array::
~array()
{
    if(tab_)
        table::destroy(tab_, sp_);
}

array::
array()
    : sp_(default_storage())
{
}

array::
array(storage_ptr store)
    : sp_(std::move(store))
{
}

array::
array(
    size_type count)
    : array(
        count, kind::null,
        default_storage())
{
}

array::
array(
    size_type count,
    storage_ptr store)
    : array(
        count, kind::null,
        std::move(store))
{
}

array::
array(
    size_type count,
    value_type const& v)
    : array(
        count, v,
        default_storage())
{
}

array::
array(
    size_type count,
    value_type const& v,
    storage_ptr store)
    : sp_(std::move(store))
{
    resize(count, v);
}

array::
array(array const& other)
    : sp_(other.get_storage())
{
    *this = other;
}

array::
array(
    array const& other,
    storage_ptr store)
    : sp_(std::move(store))
{
    *this = other;
}

array::
array(array&& other) noexcept
    : tab_(boost::exchange(
        other.tab_, nullptr))
    , sp_(other.sp_)
{
}

array::
array(
    array&& other,
    storage_ptr store)
    : sp_(std::move(store))
{
    *this = std::move(other);
}

array::
array(
    std::initializer_list<value_type> list)
    : sp_(default_storage())
{
    *this = list;
}

array::
array(
    std::initializer_list<value_type> list,
    storage_ptr store)
    : sp_(std::move(store))
{
    *this = list;
}

array&
array::
operator=(array&& other)
{
    if(*sp_ == *other.sp_)
    {
        if(tab_)
            table::destroy(tab_, sp_);
        tab_ = boost::exchange(
            other.tab_, nullptr);
    }
    else
    {
        *this = other;
    }
    return *this;
}

array&
array::
operator=(array const& other)
{
    cleanup_assign c(*this);
    reserve(other.size());
    for(auto const& v : other)
        emplace_impl(end(), v);
    c.ok = true;
    return *this;
}

array&
array::
operator=(
    std::initializer_list<value_type> list)
{
    cleanup_assign c(*this);
    reserve(list.size());
    for(auto it = list.begin();
            it != list.end(); ++it)
        emplace_impl(end(), std::move(*it));
    c.ok = true;
    return *this;
}

//------------------------------------------------------------------------------
//
// Elements
//
//------------------------------------------------------------------------------

auto
array::
at(size_type pos) ->
    reference
{
    if(pos >= size())
        throw std::out_of_range(
            "json::array index out of bounds");
    return tab_->begin()[pos];
}

auto
array::
at(size_type pos) const ->
    const_reference
{
    if(pos >= size())
        throw std::out_of_range(
            "json::array index out of bounds");
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
data() noexcept ->
    value_type*
{
    if(! tab_)
        return nullptr;
    return tab_->begin();
}

auto
array::
data() const noexcept ->
    value_type const*
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

//------------------------------------------------------------------------------
//
// Capacity
//
//------------------------------------------------------------------------------

bool
array::
empty() const noexcept
{
    return ! tab_ || tab_->size == 0;
}

auto
array::
size() const noexcept ->
    size_type
{
    if(! tab_)
        return 0;
    return tab_->size;
}

auto
array::
max_size() const noexcept ->
    size_type
{
    return (std::numeric_limits<
        size_type>::max)() / sizeof(value_type);
}

void
array::
reserve(size_type new_capacity)
{
    // minimum size is 3
    if( new_capacity > 0 &&
        new_capacity < 3)
        new_capacity = 3;

    // don't shrink
    if( tab_ &&
        new_capacity <= tab_->capacity())
        return;
    new_capacity = (std::max<size_type>)(
        tab_ ? ((tab_->capacity() * 3 + 1) / 2) : 0,
        new_capacity);

    auto tab = table::create(new_capacity, sp_);
    if(! tab_)
    {
        tab_ = tab;
        return;
    }

    for(size_type i = 0; i < tab_->size; ++i)
        ::new(&tab->begin()[i]) value_type(
            std::move(tab_->begin()[i]));
    tab->size = tab_->size;
    std::swap(tab, tab_);
    table::destroy(tab, sp_);
}

auto
array::
capacity() const noexcept ->
    size_type
{
    if(! tab_)
        return 0;
    return tab_->capacity();
}

void
array::
shrink_to_fit() noexcept
{
    if(! tab_ ||
        tab_->capacity() <= tab_->size)
        return;
    auto tab = table::create(tab_->size, sp_);
    for(size_type i = 0; i < tab_->size; ++i)
        ::new(&tab->begin()[i]) value_type(
            std::move(tab_->begin()[i]));
    tab->size = tab_->size;
    std::swap(tab, tab_);
    table::destroy(tab, sp_);
}

//------------------------------------------------------------------------------
//
// Modifiers
//
//------------------------------------------------------------------------------

void
array::
clear() noexcept
{
    if(! tab_)
        return;
    destroy(begin(), end());
    tab_->size = 0;
}

auto
array::
insert(
    const_iterator before,
    value_type const& v) ->
        iterator
{
    return emplace_impl(before, v);
}

auto
array::
insert(
    const_iterator before,
    value_type&& v) ->
        iterator
{
    return emplace_impl(
        before, std::move(v));
}

auto
array::
insert(
    const_iterator before,
    size_type count,
    value_type const& v) ->
        iterator
{
    auto pos = before - begin();
    reserve(size() + count);
    cleanup_insert c(pos, count, *this);
    while(count--)
    {
        ::new(&begin()[pos++])
            value_type(v, sp_);
        ++c.valid;
    }
    c.ok = true;
    return begin() + c.pos;
}

auto
array::
insert(
    const_iterator before,
    std::initializer_list<value_type> list) ->
        iterator
{
    auto pos = before - begin();
    reserve(size() + list.size());
    cleanup_insert c(
        pos, list.size(), *this);
    for(auto it = list.begin();
        it != list.end(); ++it)
    {
        ::new(&begin()[pos++]) value_type(
            std::move(*it), sp_);
        ++c.valid;
    }
    c.ok = true;
    return begin() + c.pos;
}

auto
array::
erase(const_iterator pos) ->
    iterator
{
    auto it = data() + (pos - begin());
    destroy(it, it + 1);
    move(it, it + 1, 1);
    --tab_->size;
    return it;
}

auto
array::
erase(
    const_iterator first,
    const_iterator last) ->
        iterator
{
    auto const n = last - first;
    auto it = data() + (first - begin());
    destroy(it, it + n);
    move(it, it + n, n);
    tab_->size -= n;
    return it;
}

void
array::
push_back(value_type const& v)
{
    emplace_impl(end(), v);
}

void
array::
push_back(value_type&& v)
{
    emplace_impl(end(), std::move(v));
}

void
array::
pop_back()
{
    back().~value();
    --tab_->size;
}

void
array::
resize(size_type count)
{
    resize(count, kind::null);
}

void
array::
resize(
    size_type count,
    value_type const& v)
{
    if(count > size())
    {
        reserve(count);
        while(count--)
            emplace_impl(end(), v);
    }
    else if(count < size())
    {
        tab_->size = count;
        count = size() - count;
        for(size_type i = size() - 1;
            count-- > 0; --i)
            tab_->begin()[i].~value();
    }
}

void
array::
swap(array& other) noexcept
{
    BOOST_ASSERT(*sp_ == *other.sp_);
    std::swap(tab_, other.tab_);
}

//------------------------------------------------------------------------------

void
array::
destroy(
    value* first,
    value* last)
{
    while(first != last)
        (*first++).~value();
}

void
array::
move(
    value* to,
    value* from,
    size_type n) noexcept
{
    if(to > from)
    {
        // backwards
        to += n;
        from += n;
        while(n--)
        {
            ::new(&*--to) value_type(
                std::move(*--from));
            from->~value();
        }
    }
    else
    {
        while(n--)
        {
            ::new(&*to++) value_type(
                std::move(*from));
            (*from++).~value();
        }
    }
}

} // json
} // boost

#endif
