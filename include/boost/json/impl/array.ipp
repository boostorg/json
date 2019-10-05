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
#include <boost/pilfer.hpp>
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

auto
array::
table::
create(
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
    table* tab,
    storage_ptr const& sp) noexcept
{
    destroy(tab->begin(), tab->end());
    sp->deallocate(tab,
        sizeof(table) +
            tab->d.capacity * sizeof(value),
        alignof(value));
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
    value* first,
    value* last) noexcept
{
    while(first != last)
        boost::relocate(dest++, *first++);
}

//------------------------------------------------------------------------------

struct array::undo
{
    table* tab;
    storage_ptr const& sp;

    ~undo()
    {
        if(tab)
            table::destroy(tab, sp);
    }
};

//------------------------------------------------------------------------------

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
    self.move(
        it + n, it,
        self.size() - pos);
    self.tab_->d.size += n;
}

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
        self.move(
            first, first + n,
            self.size() - pos);
    }
}

template<class Arg>
void
array::
undo_insert::
emplace(Arg&& arg)
{
    ::new(it) value(
        std::forward<Arg>(arg),
        self.sp_);
    ++it;
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
    value const& v)
    : array(
        count,
        v,
        default_storage())
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

    undo u{table::create(
        count, sp_), sp_};
    while(count--)
    {
        ::new(u.tab->end()) value(v, sp_);
        ++u.tab->d.size;
    }
    std::swap(tab_, u.tab);
}

array::
array(
    size_type count)
    : array(
        count,
        value(kind::null),
        default_storage())
{
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
    std::initializer_list<value> init)
    : array(
        init,
        default_storage())
{
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
operator=(array const& other)
{
    copy(other);
    return *this;
}

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
            table::destroy(tab_, sp_);
        tab_ = boost::exchange(
            other.tab_, nullptr);
    }
    return *this;
}

array&
array::
operator=(
    std::initializer_list<value> init)
{
    assign(init);
    return *this;
}

storage_ptr const&
array::
get_storage() const noexcept
{
    return sp_;
}

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
        BOOST_THROW_EXCEPTION(std::out_of_range(
            "json::array index out of bounds"));
    return tab_->begin()[pos];
}

auto
array::
at(size_type pos) const ->
    const_reference
{
    if(pos >= size())
        BOOST_THROW_EXCEPTION(std::out_of_range(
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
        table::create(new_capacity, sp_);
    if(! tab_)
    {
        tab_ = tab;
        return;
    }

    table::relocate(
        tab->begin(),
        tab_->begin(), tab_->end());
    tab->d.size = tab_->d.size;
    tab_->d.size = 0;
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
    return tab_->d.capacity;
}

void
array::
shrink_to_fit() noexcept
{
    if(capacity() <= size())
        return;
    if(tab_->d.size == 0)
    {
        table::destroy(tab_, sp_);
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
        tab = table::create(
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
        tab_->begin(), tab_->end());
    tab->d.size = tab_->d.size;
    tab_->d.size = 0;
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
    table::destroy(
        tab_->begin(),
        tab_->end());
    tab_->d.size = 0;
}

auto
array::
insert(
    const_iterator pos,
    value const& v) ->
        iterator
{
    return emplace_impl(pos, v);
}

auto
array::
insert(
    const_iterator pos,
    value&& v) ->
        iterator
{
    return emplace_impl(
        pos, std::move(v));
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
    move(p, p + 1, 1);
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
    move(p, p + n,
        size() - (last - begin()));
    tab_->d.size -= n;
    return p;
}

void
array::
push_back(value const& v)
{
    emplace_impl(end(), v);
}

void
array::
push_back(value&& v)
{
    emplace_impl(end(), std::move(v));
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
    other.~array();
    ::new(&other) array(pilfer(temp1));
    this->~array();
    ::new(this) array(pilfer(temp2));
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
        table::destroy(tab_, sp_);
        tab_ = nullptr;
        return;
    }

    undo u{table::create(
        other.size(), sp_), sp_};
    for(auto const& v : other)
    {
        ::new(u.tab->end()) value(v, sp_);
        ++u.tab->d.size;
    }
    std::swap(tab_, u.tab);
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
            boost::relocate(
                --to, *--from);
    }
    else
    {
        while(n--)
            boost::relocate(
                to++, *from++);
    }
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
        table::destroy(tab_, sp_);
        tab_ = nullptr;
        return;
    }

    undo u{table::create(
        init.size(), sp_), sp_};
    for(auto const& v : init)
    {
        ::new(u.tab->end()) value(v, sp_);
        ++u.tab->d.size;
    }
    std::swap(tab_, u.tab);
}

} // json
} // boost

#endif
