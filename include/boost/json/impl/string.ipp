//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_STRING_IPP
#define BOOST_JSON_IMPL_STRING_IPP

#include <boost/json/detail/assert.hpp>
#include <algorithm>
#include <new>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace boost {
namespace json {

//----------------------------------------------------------

string::
impl::
impl(
    size_type size_,
    storage_ptr const& sp)
{
    if(size_ < sizeof(buf))
    {
        // SBO
        capacity = sizeof(buf) - 1;
    }
    else
    {
        capacity = growth(size_,
            sizeof(buf) - 1);
        p = static_cast<char*>(
            sp->allocate(capacity + 1, 1));
    }
    size = static_cast<
        impl_size_type>(size_);
}

auto
string::
impl::
growth(
    size_type new_size,
    impl_size_type capacity) ->
        impl_size_type
{
    if(new_size > max_size())
        BOOST_JSON_THROW(
            std::length_error(
                "size > max_size()"));
    new_size |= mask_;
    if( new_size > max_size())
        return detail::max_string_length_;
    // growth factor 2
    if( capacity >
        detail::max_string_length_ - capacity)
        return detail::max_string_length_; // overflow
    return (std::max<impl_size_type>)(
        capacity * 2, static_cast<
            impl_size_type>(new_size));
}

void
string::
impl::
destroy(
    storage_ptr const& sp) noexcept
{
    if(! in_sbo())
        sp->deallocate(
            p, capacity + 1, 1);
}

char*
string::
impl::
assign(
    size_type new_size,
    storage_ptr const& sp)
{
    if(new_size > capacity)
    {
        impl tmp(growth(
            new_size,
            capacity), sp);
        destroy(sp);
        *this = tmp;
    }
    term(new_size);
    return data();
}

char*
string::
impl::
append(
    size_type n,
    storage_ptr const& sp)
{
    if(n > max_size() - size)
        BOOST_JSON_THROW(
            std::length_error(
                "size > max_size()"));
    if(n <= capacity - size)
    {
        term(size + n);
        return end() - n;
    }
    impl tmp(growth(
        size + n, capacity), sp);
    traits_type::copy(
        tmp.data(), data(), size);
    tmp.term(size + n);
    destroy(sp);
    *this = tmp;
    return end() - n;
}

char*
string::
impl::
insert(
    size_type pos,
    size_type n,
    storage_ptr const& sp)
{
    if(pos > size)
        BOOST_JSON_THROW(
            std::out_of_range(
                "pos > size()"));
    if(n <= capacity - size)
    {
        auto const dest =
            data() + pos;
        traits_type::move(
            dest + n,
            dest,
            size + 1 - pos);
        size += static_cast<
            impl_size_type>(n);
        return dest;
    }
    if(n > max_size() - size)
        BOOST_JSON_THROW(
            std::length_error(
                "size > max_size()"));
    impl tmp(growth(
        size + n, capacity), sp);
    tmp.size = size + static_cast<
        impl_size_type>(n);
    traits_type::copy(
        tmp.data(),
        data(),
        pos);
    traits_type::copy(
        tmp.data() + pos + n,
        data() + pos,
        size + 1 - pos);
    destroy(sp);
    *this = tmp;
    return data() + pos;
}

void
string::
impl::
unalloc(storage_ptr const& sp) noexcept
{
    BOOST_JSON_ASSERT(size < sizeof(buf));
    BOOST_JSON_ASSERT(! in_sbo());
    auto const p_ = p;
    traits_type::copy(
        buf, data(), size + 1);
    sp->deallocate(
        p_, capacity + 1, 1);
    capacity = sizeof(buf) - 1;
}

//----------------------------------------------------------
//
// Assignment
//
//----------------------------------------------------------

string&
string::
assign(
    size_type count,
    char ch)
{
    traits_type::assign(
        impl_.assign(count, sp_),
        count,
        ch);
    return *this;
}

string&
string::
assign(
    string const& other)
{
    if(this == &other)
        return *this;
    return assign(
        other.data(),
        other.size());
}

string&
string::
assign(string&& other)
{
    if(*sp_ == *other.sp_)
    {
        impl_.destroy(sp_);
        impl_ = other.impl_;
        ::new(&other.impl_) impl();
        return *this;
    }

    // copy
    return assign(other);
}

string&
string::
assign(
    char const* s,
    size_type count)
{
    traits_type::copy(
        impl_.assign(count, sp_),
        s, count);
    return *this;
}

//----------------------------------------------------------
//
// Capacity
//
//----------------------------------------------------------

void
string::
shrink_to_fit()
{
    if(impl_.in_sbo())
        return;
    if(impl_.size < sizeof(impl_.buf))
    {
        impl_.unalloc(sp_);
        return;
    }
    auto const new_cap =
        (std::min<size_type>)(
            impl_.size | mask_,
            max_size());
    if(new_cap >= impl_.capacity)
        return;

    impl tmp(new_cap, sp_);
    traits_type::copy(tmp.data(),
        impl_.data(), impl_.size + 1);
    tmp.size = impl_.size;
    impl_.destroy(sp_);
    impl_ = tmp;
}

//----------------------------------------------------------
//
// Operations
//
//----------------------------------------------------------

void
string::
clear() noexcept
{
    impl_.term(0);
}

//----------------------------------------------------------

void
string::
push_back(char ch)
{
    *impl_.append(1, sp_) = ch;
}

void
string::
pop_back()
{
    impl_.data()[--impl_.size] = 0;
}

//----------------------------------------------------------

string&
string::
append(size_type count, char ch)
{
    traits_type::assign(
        impl_.append(count, sp_),
        count, ch);
    return *this;
}

string&
string::
append(
    char const* s,
    size_type count)
{
    traits_type::copy(
        impl_.append(count, sp_),
        s, count);
    return *this;
}

//----------------------------------------------------------

string&
string::
insert(
    size_type pos,
    size_type count,
    char ch)
{
    traits_type::assign(
        impl_.insert(pos, count, sp_),
        count, ch);
    return *this;
}

string&
string::
insert(
    size_type pos,
    char const* s,
    size_type count)
{
    if(pos >= impl_.size)
        BOOST_JSON_THROW(
            std::out_of_range(
                "pos >= size()"));
    if(count > impl_.capacity - impl_.size)
    {
        traits_type::copy(
            impl_.insert(pos, count, sp_),
            s, count);
        return *this;
    }
    // VFALCO TODO handle [s, s+count) inside *this
    traits_type::move(
        impl_.data() + pos + count,
        impl_.data() + pos,
        impl_.size - pos + 1);
    traits_type::copy(
        impl_.data() + pos,
        s, count);
    impl_.size += static_cast<
        impl_size_type>(count);
    return *this;
}

auto
string::
insert(
    const_iterator pos,
    size_type count,
    char ch) ->
        iterator
{
    auto const off = pos - begin();
    insert(off, count, ch);
    return begin() + off;
}

auto
string::
insert(
    const_iterator pos,
    std::initializer_list<char> init) ->
        iterator
{
    auto const off = pos - begin();
    insert(off, init.begin(), init.size());
    return begin() + off;
}


//----------------------------------------------------------

string&
string::
erase(
    size_type pos,
    size_type count)
{
    if(pos > impl_.size)
        BOOST_JSON_THROW(
            std::out_of_range(
                "pos > size()"));
    if( count > impl_.size - pos)
        count = impl_.size - pos;
    traits_type::move(
        impl_.data() + pos,
        impl_.data() + pos + count,
        impl_.size - pos - count + 1);
    impl_.size -= static_cast<
        impl_size_type>(count);
    impl_.data()[impl_.size] = 0;
    return *this;
}

auto
string::
erase(const_iterator pos) ->
    iterator
{
    return erase(pos, pos+1);
}

auto
string::
erase(
    const_iterator first,
    const_iterator last) ->
        iterator
{
    auto const pos = first - begin();
    auto const count = last - first;
    erase(pos, count);
    return data() + pos;
}

//----------------------------------------------------------

void
string::
resize(size_type count, char ch)
{
    if(count <= impl_.size)
    {
        auto const n = static_cast<
            impl_size_type>(count);
        impl_.data()[n] = 0;
        impl_.size = n;
        return;
    }

    reserve(count);
    traits_type::assign(
        impl_.end(),
        count + 1 - impl_.size,
        ch);
    impl_.size = static_cast<
        impl_size_type>(count);
}

//----------------------------------------------------------

void
string::
swap(string& other)
{
    if(*sp_ == *other.sp_)
    {
        std::swap(impl_, other.impl_);
        return;
    }

    string temp1(
        std::move(*this), other.sp_);
    string temp2(
        std::move(other), sp_);
    this->~string();
    ::new(this) string(pilfer(temp2));
    other.~string();
    ::new(&other) string(pilfer(temp1));
}

//----------------------------------------------------------

void
string::
reserve_impl(size_type new_cap)
{
    BOOST_JSON_ASSERT(
        new_cap >= impl_.capacity);
    if(new_cap > impl_.capacity)
    {
        // grow
        new_cap = impl::growth(
            new_cap, impl_.capacity);
        impl tmp(new_cap, sp_);
        traits_type::copy(tmp.data(),
            impl_.data(), impl_.size + 1);
        tmp.size = impl_.size;
        impl_.destroy(sp_);
        impl_ = tmp;
        return;
    }
    /*
    if( new_cap >= sizeof(impl_.buf) ||
        impl_.in_sbo())
    {
        // do nothing
        return;
    }
    impl_.unalloc(sp_);
    */
}

storage_ptr
string::
release_storage() noexcept
{
    impl_.destroy(sp_);
    return std::move(sp_);
}

//----------------------------------------------------------

std::ostream&
operator<<(std::ostream& os, string const& s)
{
    return os << static_cast<string_view>(s);
}

} // json
} // boost

#endif
