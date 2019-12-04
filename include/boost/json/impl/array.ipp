//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_ARRAY_IPP
#define BOOST_JSON_IMPL_ARRAY_IPP

#include <boost/json/array.hpp>
#include <boost/json/detail/except.hpp>
#include <boost/pilfer.hpp>
#include <cstdlib>
#include <limits>
#include <new>
#include <utility>

namespace boost {
namespace json {

array::
undo_construct::
~undo_construct()
{
    if(! commit)
        self_.impl_.destroy(self_.sp_);
}

//----------------------------------------------------------

array::
undo_insert::
undo_insert(
    value const* pos_,
    std::size_t n,
    array& self)
    : self_(self)
    , n_(static_cast<std::size_t>(n))
    , pos(self.impl_.index_of(pos_))
{
    if(n > max_size())
        BOOST_JSON_THROW(
            detail::array_too_large_exception());
    self_.reserve(
        self_.impl_.size() + n_);
    // (iterators invalidated now)
    it = self_.impl_.data() + pos;
    relocate(it + n_, it,
        self_.impl_.size() - pos);
    self_.impl_.size(
        self_.impl_.size() + n_);
}

array::
undo_insert::
~undo_insert()
{
    if(! commit)
    {
        auto const first =
            self_.impl_.data() + pos;
        self_.destroy(first, it);
        self_.impl_.size(
            self_.impl_.size() - n_);
        relocate(
            first, first + n_,
            self_.impl_.size() - pos);
    }
}

//----------------------------------------------------------
//
// Special Members
//
//----------------------------------------------------------

array::
array(detail::unchecked_array&& ua)
    : sp_(ua.storage())
    , impl_(ua.size(), sp_) // exact
{
    impl_.size(ua.size());
    ua.relocate(impl_.data());
}

array::
array(storage_ptr sp) noexcept
    : sp_(std::move(sp))
{
    // silence -Wunused-private-field
    k_ = kind::array;
}

array::
array(
    std::size_t count,
    value const& v,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    undo_construct u(*this);
    reserve(count);
    while(impl_.size() < count)
    {
        ::new(
            impl_.data() +
            impl_.size()) value(v, sp_);
        impl_.size(impl_.size() + 1);
    }
    u.commit = true;
}

array::
array(
    std::size_t count,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    undo_construct u(*this);
    reserve(count);
    while(impl_.size() < count)
    {
        ::new(
            impl_.data() +
            impl_.size()) value(sp_);
        impl_.size(impl_.size() + 1);
    }
    u.commit = true;
}

array::
array(array const& other)
    : sp_(other.sp_)
{
    undo_construct u(*this);
    copy(other);
    u.commit = true;
}

array::
array(
    array const& other,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    undo_construct u(*this);
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
        undo_construct u(*this);
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
    undo_construct u(*this);
    copy(init);
    u.commit = true;
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
    if(impl_.capacity() <= impl_.size())
        return;
    if(impl_.size() == 0)
    {
        impl_.destroy(sp_);
        impl_ = {};
        return;
    }
    if( impl_.size() <
            array_impl::min_capacity &&
        impl_.capacity() <=
            array_impl::min_capacity)
        return;

#ifndef BOOST_NO_EXCEPTIONS
    try
    {
#endif
        array_impl impl(impl_.size(), sp_);
        relocate(
            impl.data(),
            impl_.data(), impl_.size());
        impl.size(impl_.size());
        impl_.size(0);
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
    if(! impl_.data())
        return;
    destroy(impl_.data(),
        impl_.data() + impl_.size());
    impl_.size(0);
}

auto
array::
insert(
    const_iterator pos,
    std::size_t count,
    value const& v) ->
        iterator
{
    undo_insert u(pos, count, *this);
    while(count--)
        u.emplace(v);
    u.commit = true;
    return impl_.data() + u.pos;
}

auto
array::
insert(
    const_iterator pos,
    std::initializer_list<value> init) ->
        iterator
{
    undo_insert u(pos,
        static_cast<std::size_t>(
            init.size()), *this);
    for(auto const& v : init)
        u.emplace(v);
    u.commit = true;
    return impl_.data() + u.pos;
}

auto
array::
erase(
    const_iterator pos) noexcept ->
    iterator
{
    auto p = impl_.data() +
        (pos - impl_.data());
    destroy(p, p + 1);
    relocate(p, p + 1, 1);
    impl_.size(impl_.size() - 1);
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
        std::size_t>(last - first);
    auto const p =
        impl_.data() + impl_.index_of(first);
    destroy(p, p + n);
    relocate(p, p + n,
        impl_.size() -
            impl_.index_of(last));
    impl_.size(impl_.size() - n);
    return p;
}

void
array::
pop_back() noexcept
{
    auto const p = &back();
    destroy(p, p + 1);
    impl_.size(impl_.size() - 1);
}

void
array::
resize(std::size_t count)
{
    if(count <= impl_.size())
    {
        destroy(
            impl_.data() + count,
            impl_.data() + impl_.size());
        impl_.size(count);
        return;
    }

    reserve(count);
    auto it =
        impl_.data() + impl_.size();
    auto const end =
        impl_.data() + count;
    while(it != end)
        ::new(it++) value(sp_);
    impl_.size(count);
}

void
array::
resize(
    std::size_t count,
    value const& v)
{
    if(count <= size())
    {
        destroy(
            impl_.data() + count,
            impl_.data() + impl_.size());
        impl_.size(count);
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
        impl_.data() + count;
    undo u{*this,
        impl_.data() + impl_.size(),
        impl_.data() + impl_.size()};
    do
    {
        ::new(u.it) value(v, sp_);
        ++u.it;
    }
    while(u.it != end);
    impl_.size(count);
    u.it = nullptr;
}

void
array::
swap(array& other)
{
    BOOST_ASSERT(this != &other);
    if(*sp_ == *other.sp_)
    {
        impl_.swap(other.impl_);
        return;
    }
    array temp1(
        std::move(*this),
        other.storage());
    array temp2(
        std::move(other),
        this->storage());
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
    impl_.destroy_impl(sp_);
}

void
array::
copy(array const& other)
{
    reserve(other.size());
    for(auto const& v : other)
    {
        ::new(
            impl_.data() +
            impl_.size()) value(v, sp_);
        impl_.size(impl_.size() + 1);
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
        std::size_t>(init.size()));
    for(auto const& v : init)
    {
        ::new(
            impl_.data() +
            impl_.size()) value(v, sp_);
        impl_.size(impl_.size() + 1);
    }
}

void
array::
reserve_impl(std::size_t capacity)
{
    if(impl_.data())
    {
        // 2x growth
        auto const hint =
            impl_.capacity() * 2;
        if(hint < impl_.capacity()) // overflow
            capacity = max_size();
        else if(capacity < hint)
            capacity = hint;
    }
    if( capacity < array_impl::min_capacity)
        capacity = array_impl::min_capacity;
    array_impl impl(capacity, sp_);
    relocate(
        impl.data(),
        impl_.data(), impl_.size());
    impl.size(impl_.size());
    impl_.size(0);
    impl_.destroy(sp_);
    impl_ = impl;
}

void
array::
relocate(
    value* dest,
    value* src,
    std::size_t n) noexcept
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
