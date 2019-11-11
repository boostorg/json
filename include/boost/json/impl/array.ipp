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
#include <boost/json/detail/exchange.hpp>
#include <boost/json/detail/assert.hpp>
#include <boost/pilfer.hpp>
#include <cstdlib>
#include <limits>
#include <new>
#include <utility>

namespace boost {
namespace json {

//----------------------------------------------------------

array::
impl_type::
impl_type(
    size_type capacity_,
    storage_ptr const& sp)
{
    // The choice of minimum capacity
    // affects the speed of parsing.
//    if( capacity_ < min_capacity_)
//        capacity_ = min_capacity_;
    if(capacity_ > 0)
        vec = reinterpret_cast<value*>(
            sp->allocate(
                capacity_ * sizeof(value),
                alignof(value)));
    else
        vec = nullptr;
    capacity = capacity_;
    size = 0;
}


array::
impl_type::
impl_type(impl_type&& other) noexcept
    : vec(detail::exchange(
        other.vec, nullptr))
    , size(detail::exchange(
        other.size, 0))
    , capacity(detail::exchange(
        other.capacity, 0))
{
}

auto
array::
impl_type::
operator=(
    impl_type&& other) noexcept ->
        impl_type&
{
    ::new(this) impl_type(
        std::move(other));
    return *this;
}


void
array::
impl_type::
swap(impl_type& rhs) noexcept
{
    std::swap(vec, rhs.vec);
    std::swap(size, rhs.size);
    std::swap(capacity, rhs.capacity);
}

void
array::
impl_type::
destroy(
    storage_ptr const& sp) noexcept
{
    if(vec && sp->need_free())
    {
        auto it = vec + size;
        while(it != vec)
            (*--it).~value();
        sp->deallocate(vec,
            capacity * sizeof(value),
            alignof(value));
    }
}

//----------------------------------------------------------

array::
undo_create::
~undo_create()
{
    if(! commit)
        self_.impl_.destroy(self_.sp_);
}

//----------------------------------------------------------

array::
undo_assign::
undo_assign(array& self)
    : self_(self)
    , impl_(std::move(self.impl_))
{
}

array::
undo_assign::
~undo_assign()
{
    if(! commit)
        impl_.swap(self_.impl_);
    impl_.destroy(self_.sp_);
}

//----------------------------------------------------------

array::
undo_insert::
undo_insert(
    value const* pos_,
    unsigned long long n,
    array& self)
    : self_(self)
    , n_(static_cast<size_type>(n))
    , pos(self.impl_.index_of(pos_))
{
    if(n > max_size())
        BOOST_JSON_THROW(
            std::length_error(
                "size > max_size()"));
    self_.reserve(
        self_.impl_.size + n_);
    // (iterators invalidated now)
    it = self_.impl_.vec + pos;
    relocate(it + n_, it,
        self_.impl_.size - pos);
    self_.impl_.size += n_;
}

array::
undo_insert::
~undo_insert()
{
    if(! commit)
    {
        auto const first =
            self_.impl_.vec + pos;
        self_.destroy(first, it);
        self_.impl_.size -= n_;
        relocate(
            first, first + n_,
            self_.impl_.size - pos);
    }
}

//----------------------------------------------------------
//
// Special Members
//
//----------------------------------------------------------

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
    undo_create u(*this);
    reserve(count);
    while(impl_.size < count)
    {
        ::new(
            impl_.vec +
            impl_.size) value(v, sp_);
        ++impl_.size;
    }
    u.commit = true;
}

array::
array(
    size_type count,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    undo_create u(*this);
    reserve(count);
    while(impl_.size < count)
    {
        ::new(
            impl_.vec +
            impl_.size) value(sp_);
        ++impl_.size;
    }
    u.commit = true;
}

array::
array(array const& other)
    : sp_(other.sp_)
{
    undo_create u(*this);
    copy(other);
    u.commit = true;
}

array::
array(
    array const& other,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    undo_create u(*this);
    copy(other);
    u.commit = true;
}

array::
array(pilfered<array> other) noexcept
    : sp_(detail::exchange(
        other.get().sp_, nullptr))
    , impl_(std::move(other.get().impl_))
{
}

array::
array(array&& other) noexcept
    : sp_(other.sp_)
    , impl_(std::move(other.impl_))
{
}

array::
array(
    array&& other,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    if(*sp_ == *other.sp_)
    {
        impl_.swap(other.impl_);
    }
    else
    {
        undo_create u(*this);
        copy(other);
        u.commit = true;
    }
}

array::
array(
    std::initializer_list<value> init,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    undo_create u(*this);
    copy(init);
    u.commit = true;
}

array::
array(unchecked_array&& ua)
    : sp_(ua.get_storage())
    , impl_(ua.size(), sp_) // exact
{
    impl_.size = ua.size();
    ua.relocate(impl_.vec);
}

//----------------------------------------------------------

array&
array::
operator=(array const& other)
{
    if(this == &other)
        return *this;
    array tmp(other, sp_);
    this->~array();
    ::new(this) array(pilfer(tmp));
    return *this;
}

array&
array::
operator=(array&& other)
{
    array tmp(std::move(other), sp_);
    this->~array();
    ::new(this) array(pilfer(tmp));
    return *this;
}

array&
array::
operator=(
    std::initializer_list<value> init)
{
    array tmp(init, sp_);
    this->~array();
    ::new(this) array(pilfer(tmp));
    return *this;
}

//----------------------------------------------------------
//
// Capacity
//
//----------------------------------------------------------

void
array::
shrink_to_fit() noexcept
{
    if(impl_.capacity <= impl_.size)
        return;
    if(impl_.size == 0)
    {
        impl_.destroy(sp_);
        impl_ = {};
        return;
    }
    if( impl_.size < min_capacity_ &&
        impl_.capacity <= min_capacity_)
        return;

#ifndef BOOST_NO_EXCEPTIONS
    try
    {
#endif
        impl_type impl(impl_.size, sp_);
        relocate(
            impl.vec,
            impl_.vec, impl_.size);
        impl.size = impl_.size;
        impl_.size = 0;
        impl_.swap(impl);
        impl.destroy(sp_);
#ifndef BOOST_NO_EXCEPTIONS
    }
    catch(...)
    {
        // eat the exception
        return;
    }
#endif
}

//----------------------------------------------------------
//
// Modifiers
//
//----------------------------------------------------------

void
array::
clear() noexcept
{
    if(! impl_.vec)
        return;
    destroy(impl_.vec,
        impl_.vec + impl_.size);
    impl_.size = 0;
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
    return impl_.vec + u.pos;
}

auto
array::
insert(
    const_iterator pos,
    std::initializer_list<value> init) ->
        iterator
{
    undo_insert u(pos,
        static_cast<size_type>(
            init.size()), *this);
    for(auto const& v : init)
        u.emplace(v);
    u.commit = true;
    return impl_.vec + u.pos;
}

auto
array::
erase(
    const_iterator pos) noexcept ->
    iterator
{
    auto p = impl_.vec +
        (pos - impl_.vec);
    destroy(p, p + 1);
    relocate(p, p + 1, 1);
    --impl_.size;
    return p;
}

auto
array::
erase(
    const_iterator first,
    const_iterator last) noexcept ->
        iterator
{
    auto const n = static_cast<
        size_type>(last - first);
    auto const p =
        impl_.vec + impl_.index_of(first);
    destroy(p, p + n);
    relocate(p, p + n,
        impl_.size -
            impl_.index_of(last));
    impl_.size -= n;
    return p;
}

void
array::
pop_back() noexcept
{
    auto const p = &back();
    destroy(p, p + 1);
    --impl_.size;
}

void
array::
resize(size_type count)
{
    if(count <= impl_.size)
    {
        destroy(
            impl_.vec + count,
            impl_.vec + impl_.size);
        impl_.size = count;
        return;
    }

    reserve(count);
    auto it =
        impl_.vec + impl_.size;
    auto const end =
        impl_.vec + count;
    while(it != end)
        ::new(it++) value(sp_);
    impl_.size = count;
}

void
array::
resize(
    size_type count,
    value const& v)
{
    if(count <= size())
    {
        destroy(
            impl_.vec + count,
            impl_.vec + impl_.size);
        impl_.size = count;
        return;
    }

    reserve(count);

    struct undo
    {
        array& self;
        value* it;
        value* const end;

        ~undo()
        {
            if(it)
                self.destroy(end, it);
        }
    };

    auto const end =
        impl_.vec + count;
    undo u{*this,
        impl_.vec + impl_.size,
        impl_.vec + impl_.size};
    do
    {
        ::new(u.it) value(v, sp_);
        ++u.it;
    }
    while(u.it != end);
    impl_.size = count;
    u.it = nullptr;
}

void
array::
swap(array& other)
{
    if(*sp_ == *other.sp_)
    {
        impl_.swap(other.impl_);
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

//----------------------------------------------------------

void
array::
destroy(
    value* first, value* last) noexcept
{
    if(sp_->need_free())
        while(last != first)
            (*--last).~value();
}

void
array::
destroy() noexcept
{
    auto it = impl_.vec + impl_.size;
    while(it != impl_.vec)
        (*--it).~value();
    sp_->deallocate(impl_.vec,
        impl_.capacity * sizeof(value),
        alignof(value));
}

void
array::
copy(array const& other)
{
    reserve(other.size());
    for(auto const& v : other)
    {
        ::new(
            impl_.vec +
            impl_.size) value(v, sp_);
        ++impl_.size;
    }
}

void
array::
copy(
    std::initializer_list<value> init)
{
    if(init.size() > max_size())
        BOOST_JSON_THROW(
            std::length_error(
                "size > max_size()"));
    reserve(static_cast<
        size_type>(init.size()));
    for(auto const& v : init)
    {
        ::new(
            impl_.vec +
            impl_.size) value(v, sp_);
        ++impl_.size;
    }
}

void
array::
reserve_impl(size_type capacity)
{
    if(impl_.vec)
    {
        // 2x growth
        auto const hint =
            impl_.capacity * 2;
        if(hint < impl_.capacity) // overflow
            capacity = max_size();
        else if(capacity < hint)
            capacity = hint;
    }
    if( capacity < min_capacity_)
        capacity = min_capacity_;
    impl_type impl(capacity, sp_);
    relocate(
        impl.vec,
        impl_.vec, impl_.size);
    impl.size = impl_.size;
    impl_.size = 0;
    impl_.destroy(sp_);
    impl_ = impl;
}

void
array::
relocate(
    value* dest,
    value* src,
    size_type n) noexcept
{
    if(n == 0)
        return;
    std::memmove(
        reinterpret_cast<
            void*>(dest),
        reinterpret_cast<
            void const*>(src),
        n * sizeof(value));
}

} // json
} // boost

#endif
