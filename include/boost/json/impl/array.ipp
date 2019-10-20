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
#include <boost/assert.hpp>
#include <boost/pilfer.hpp>
#include <boost/static_assert.hpp>
#include <cstdlib>
#include <limits>
#include <new>
#include <utility>

namespace boost {
namespace json {

//------------------------------------------------------------------------------

void
array::
table::
destroy(
    storage_ptr const& sp) noexcept
{
    destroy(begin(), end());
    sp->deallocate(this,
        sizeof(table) +
            d.capacity * sizeof(value),
        alignof(value));
}

auto
array::
table::
construct(
    size_type capacity,
    storage_ptr const& sp) ->
        table*
{
    // a reasonable minimum
    if( capacity < 3)
        capacity = 3;

    BOOST_STATIC_ASSERT(
        sizeof(table) == sizeof(value));
    auto const p = ::new(sp->allocate(
        sizeof(table) +
            capacity * sizeof(value),
        alignof(value))) table;
    p->d.size = 0;
    p->d.capacity = capacity;
    return p;
}

void
array::
table::
destroy(
    value* first,
    value* last) noexcept
{
    while(last != first)
        (*--last).~value();
}

void
array::
table::
relocate(
    value* dest,
    value* src,
    size_type n) noexcept
{
    if( dest >= src &&
        dest < src + n)
    {
        // backwards
        dest += n;
        auto it = src + n;
        while(it != src)
            boost::relocate(
                --dest, *--it);
    }
    else
    {
        auto last = src + n;
        while(src != last)
            boost::relocate(
                dest++, *src++);
    }
}

//------------------------------------------------------------------------------

array::
undo_create::
~undo_create()
{
    if(! commit_)
    {
        if(self_.tab_)
            self_.tab_->destroy(
                self_.sp_);
        self_.tab_ = saved_;
    }
    else if(saved_)
    {
        saved_->destroy(self_.sp_);
    }
}

array::
undo_create::
undo_create(
    array& self)
    : self_(self)
{
}

array::
undo_create::
undo_create(
    size_type n,
    array& self)
    : self_(self)
    , saved_(boost::exchange(
        self.tab_,
        table::construct(n, self.sp_)))
{
}

//------------------------------------------------------------------------------

array::
undo_insert::
~undo_insert()
{
    if(! commit)
    {
        table::destroy(
            self.begin() + pos, it);
        self.tab_->d.size -= n;
        auto const first =
            self.begin() + pos;
        table::relocate(
            first, first + n,
            self.size() - pos);
    }
}

array::
undo_insert::
undo_insert(
    value const* pos_,
    size_type n_,
    array& self_)
    : self(self_)
    , pos(pos_ - self_.begin())
    , n(n_)
{
    self.reserve(self.size() + n);
    // (iterators invalidated now)
    it = self.begin() + pos;
    table::relocate(
        it + n, it,
        self.size() - pos);
    self.tab_->d.size += n;
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
        tab_->destroy(sp_);
}

//------------------------------------------------------------------------------

array::
array() noexcept
    : sp_(default_storage())
{
}

array::
array(storage_ptr sp) noexcept
    : sp_(std::move(sp))
{
}

array::
array(
    size_type count,
    value const& v,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    if(count == 0)
        return;

    undo_create u(count, *this);
    while(count--)
    {
        ::new(tab_->end()) value(v, sp_);
        ++tab_->d.size;
    }
    u.commit();
}

array::
array(
    size_type count,
    storage_ptr sp)
    : array(
        count,
        value(kind::null),
        std::move(sp))
{
}

array::
array(array const& other)
    : array(other, other.sp_)
{
}

array::
array(
    array const& other,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    copy(other);
}

array::
array(pilfered<array> other) noexcept
    : tab_(boost::exchange(
        other.get().tab_, nullptr))
    , sp_(boost::exchange(
        other.get().sp_, nullptr))
{
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
    storage_ptr sp)
    : sp_(std::move(sp))
{
    if(*sp_ != *other.sp_)
        copy(other);
    else
        std::swap(tab_, other.tab_);
}

array::
array(
    std::initializer_list<value> init,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    assign(init);
}

//------------------------------------------------------------------------------

array&
array::
operator=(array&& other)
{
    if(*sp_ != *other.sp_)
    {
        copy(other);
    }
    else
    {
        if(tab_)
            tab_->destroy(sp_);
        tab_ = boost::exchange(
            other.tab_, nullptr);
    }
    return *this;
}

//------------------------------------------------------------------------------
//
// Capacity
//
//------------------------------------------------------------------------------

void
array::
reserve(size_type new_capacity)
{
    // this is a no-op
    if(new_capacity == 0)
        return;

    if(tab_)
    {
        // never shrink
        if(new_capacity <= tab_->d.capacity)
            return;

        // grow at least 50%
        new_capacity = (std::max<size_type>)(
            (tab_->d.capacity * 3 + 1) / 2,
            new_capacity);
    }

    auto tab =
        table::construct(new_capacity, sp_);
    if(! tab_)
    {
        tab_ = tab;
        return;
    }

    table::relocate(
        tab->begin(),
        tab_->begin(),
        tab_->d.size);
    tab->d.size = tab_->d.size;
    tab_->d.size = 0;
    std::swap(tab, tab_);
    tab->destroy(sp_);
}

void
array::
shrink_to_fit() noexcept
{
    if(capacity() <= size())
        return;
    if(tab_->d.size == 0)
    {
        tab_->destroy(sp_);
        tab_ = nullptr;
        return;
    }
    if(size() < 3 && capacity() <= 3)
        return;

    table* tab;
#ifndef BOOST_NO_EXCEPTIONS
    try
    {
#endif
        tab = table::construct(
            tab_->d.size, sp_);
#ifndef BOOST_NO_EXCEPTIONS
    }
    catch(...)
    {
        // eat the exception
        return;
    }
#endif

    table::relocate(
        tab->begin(),
        tab_->begin(),
        tab_->d.size);
    tab->d.size = tab_->d.size;
    tab_->d.size = 0;
    std::swap(tab, tab_);
    tab->destroy(sp_);
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
    table::destroy(
        tab_->begin(),
        tab_->end());
    tab_->d.size = 0;
}

auto
array::
insert(
    const_iterator pos,
    size_type count,
    value const& v) ->
        iterator
{
    undo_insert u(pos, count, *this);
    while(count--)
        u.emplace(v);
    u.commit = true;
    return begin() + u.pos;
}

auto
array::
insert(
    const_iterator pos,
    std::initializer_list<value> init) ->
        iterator
{
    undo_insert u(
        pos, init.size(), *this);
    for(auto const& v : init)
        u.emplace(v);
    u.commit = true;
    return begin() + u.pos;
}

auto
array::
erase(
    const_iterator pos) noexcept ->
    iterator
{
    auto p = data() + (pos - begin());
    table::destroy(p, p + 1);
    table::relocate(p, p + 1, 1);
    --tab_->d.size;
    return p;
}

auto
array::
erase(
    const_iterator first,
    const_iterator last) noexcept ->
        iterator
{
    auto const n = last - first;
    auto p = data() + (first - begin());
    table::destroy(p, p + n);
    table::relocate(p, p + n,
        size() - (last - begin()));
    tab_->d.size -= n;
    return p;
}

void
array::
pop_back() noexcept
{
    back().~value();
    --tab_->d.size;
}

void
array::
resize(size_type count)
{
    if(count <= size())
    {
        table::destroy(
            tab_->begin() + count,
            tab_->end());
        tab_->d.size = count;
        return;
    }

    reserve(count);
    auto first = tab_->end();
    auto const last =
        tab_->begin() + count;
    while(first != last)
        ::new(first++) value(
            json::kind::null, sp_);
    tab_->d.size = count;
}

void
array::
resize(
    size_type count,
    value const& v)
{
    if(count <= size())
    {
        table::destroy(
            tab_->begin() + count,
            tab_->end());
        tab_->d.size = count;
        return;
    }

    reserve(count);

    struct revert
    {
        value* it;
        table* tab;

        ~revert()
        {
            if(it != tab->end())
                table::destroy(
                    tab->end(), it);
        }
    };

    revert r{tab_->end(), tab_};
    auto const last =
        tab_->begin() + count;
    do
    {
        ::new(r.it) value(v, sp_);
        ++r.it;
    }
    while(r.it != last);
    tab_->d.size = count;
}

void
array::
swap(array& other)
{
    if(*sp_ == *other.sp_)
    {
        std::swap(tab_, other.tab_);
        return;
    }

    array temp1(
        std::move(*this),
        other.get_storage());
    array temp2(
        std::move(other),
        this->get_storage());
    this->~array();
    ::new(this) array(pilfer(temp2));
    other.~array();
    ::new(&other) array(pilfer(temp1));
}

//------------------------------------------------------------------------------

void
array::
copy(array const& other)
{
    if(other.empty())
    {
        if(! tab_)
            return;
        tab_->destroy(sp_);
        tab_ = nullptr;
        return;
    }

    undo_create u(other.size(), *this);
    for(auto const& v : other)
    {
        ::new(tab_->end()) value(v, sp_);
        ++tab_->d.size;
    }
    u.commit();
}

void
array::
assign(
    std::initializer_list<value> init)
{
    if(init.size() == 0)
    {
        if(! tab_)
            return;
        tab_->destroy(sp_);
        tab_ = nullptr;
        return;
    }

    undo_create u(init.size(), *this);
    for(auto const& v : init)
    {
        ::new(tab_->end()) value(v, sp_);
        ++tab_->d.size;
    }
    u.commit();
}

} // json
} // boost

#endif
