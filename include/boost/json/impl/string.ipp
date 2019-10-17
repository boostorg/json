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

#include <boost/core/exchange.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/throw_exception.hpp>
#include <algorithm>
#include <new>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace boost {
namespace json {

//------------------------------------------------------------------------------

auto
string::
impl::
growth(
    size_type new_size,
    impl_size_type capacity) ->
        impl_size_type
{
    if(new_size > detail::max_string_length_)
        BOOST_THROW_EXCEPTION(
            std::length_error(
                "size > max_size()"));
    new_size |= mask_;
    if( new_size > detail::max_string_length_)
        return detail::max_string_length_;
    if( capacity >
        detail::max_string_length_ - capacity / 2)
        return detail::max_string_length_; // overflow
    return (std::max<impl_size_type>)(
        capacity + capacity / 2,
        static_cast<impl_size_type>(
            new_size));
}

void
string::
impl::
destroy(
    storage_ptr const& sp)
{
    if(! in_sbo())
        sp->deallocate(
            p, capacity + 1, 1);
}

void
string::
impl::
construct() noexcept
{
    size = 0;
    capacity = sizeof(buf) - 1;
    buf[0] = 0;
}

char*
string::
impl::
construct(
    size_type new_size,
    storage_ptr const& sp)
{
    if(new_size < sizeof(buf))
    {
        // SBO
        capacity = sizeof(buf) - 1;
        return buf;
    }

    capacity = growth(new_size,
        sizeof(buf) - 1);
    p = static_cast<char*>(
        sp->allocate(capacity + 1, 1));
    return p;
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
        impl tmp;
        tmp.construct(growth(
            new_size, capacity), sp);
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
    if(n > detail::max_string_length_ - size)
        BOOST_THROW_EXCEPTION(
            std::length_error(
                "size > max_size()"));
    if(n <= capacity - size)
    {
        term(size + n);
        return end() - n;
    }
    impl tmp;
    traits_type::copy(
        tmp.construct(growth(
            size + n, capacity), sp),
        data(), size);
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
        BOOST_THROW_EXCEPTION(
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
    if(n > detail::max_string_length_ - size)
        BOOST_THROW_EXCEPTION(
            std::length_error(
                "size > max_size()"));
    impl tmp;
    tmp.construct(growth(
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
    BOOST_ASSERT(size < sizeof(buf));
    BOOST_ASSERT(! in_sbo());
    auto const p_ = p;
    traits_type::copy(
        buf, data(), size + 1);
    sp->deallocate(
        p_, capacity + 1, 1);
    capacity = sizeof(buf) - 1;
}

//------------------------------------------------------------------------------

string::
~string()
{
    s_.destroy(sp_);
}

//------------------------------------------------------------------------------
//
// Construction
//
//------------------------------------------------------------------------------

string::
string() noexcept
    : sp_(default_storage())
{
    s_.construct();
}

string::
string(storage_ptr sp) noexcept
    : sp_(std::move(sp))
{
    s_.construct();
}

string::
string(
    size_type count,
    char ch,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    s_.construct();
    assign(count, ch);
}

string::
string(
    string const& other,
    size_type pos,
    size_type count,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    s_.construct();
    assign(other, pos, count);
}

string::
string(
    char const* s,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    s_.construct();
    assign(s);
}

string::
string(
    char const* s,
    size_type count,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    s_.construct();
    assign(s, count);
}

string::
string(string const& other)
    : sp_(default_storage())
{
    s_.construct();
    assign(other);
}

string::
string(
    string const& other,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    s_.construct();
    assign(other);
}

string::
string(pilfered<string> p) noexcept
    : s_(p.get().s_)
    , sp_(std::move(p.get().sp_))
{
    p.get().s_.construct();
}

string::
string(string&& other) noexcept
    : s_(other.s_)
    , sp_(other.sp_)
{
    other.s_.construct();
}

string::
string(
    string&& other,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    s_.construct();
    assign(std::move(other));
}

string::
string(
    std::initializer_list<char> init,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    s_.construct();
    assign(init);
}

string::
string(
    string_view s,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    s_.construct();
    assign(s);
}

string::
string(
    string_view s,
    size_type pos,
    size_type n,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    s_.construct();
    assign(s.substr(pos, n));
}

//------------------------------------------------------------------------------

string&
string::
operator=(char ch)
{
    *s_.assign(1, sp_) = ch;
    return *this;
}

//------------------------------------------------------------------------------

string&
string::
assign(
    size_type count,
    char ch)
{
    traits_type::assign(
        s_.assign(count, sp_),
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
    // copy
    if(*sp_ != *other.sp_)
        return assign(other);

    s_.destroy(sp_);
    s_ = other.s_;
    if(! other.s_.in_sbo())
    {
        other.s_.construct(0, sp_);
        other.s_.data()[0] = 0;
        other.s_.size = 0;
    }
    return *this;
}

string&
string::
assign(
    char const* s,
    size_type count)
{
    traits_type::copy(
        s_.assign(count, sp_),
        s, count);
    return *this;
}

//------------------------------------------------------------------------------
//
// Element Access
//
//------------------------------------------------------------------------------

char&
string::
at(size_type pos)
{
    if(pos >= size())
        BOOST_THROW_EXCEPTION(
            std::out_of_range(
                "pos >= size()"));
    return s_.data()[pos];
}

char const&
string::
at(size_type pos) const
{
    if(pos >= size())
        BOOST_THROW_EXCEPTION(
            std::out_of_range(
                "pos >= size()"));
    return s_.data()[pos];
}

//------------------------------------------------------------------------------
//
// Capacity
//
//------------------------------------------------------------------------------

void
string::
reserve(size_type new_cap)
{
    if(new_cap < s_.size)
        return;
    if(new_cap == s_.capacity)
        return;
    if(new_cap > s_.capacity)
    {
        // grow
        new_cap = impl::growth(
            new_cap, s_.capacity);
        impl tmp;
        tmp.construct(new_cap, sp_);
        traits_type::copy(tmp.data(),
            s_.data(), s_.size + 1);
        tmp.size = s_.size;
        s_.destroy(sp_);
        s_ = tmp;
        return;
    }
    if( new_cap >= sizeof(s_.buf) ||
        s_.in_sbo())
    {
        // do nothing
        return;
    }
    s_.unalloc(sp_);
}

void
string::
shrink_to_fit()
{
    if(s_.in_sbo())
        return;
    if(s_.size < sizeof(s_.buf))
    {
        s_.unalloc(sp_);
        return;
    }
    auto const new_cap =
        (std::min<size_type>)(
            s_.size | mask_,
            max_size());
    if(new_cap >= s_.capacity)
        return;

    impl tmp;
    tmp.construct(new_cap, sp_);
    traits_type::copy(tmp.data(),
        s_.data(), s_.size + 1);
    tmp.size = s_.size;
    s_.destroy(sp_);
    s_ = tmp;
}

//------------------------------------------------------------------------------
//
// Operations
//
//------------------------------------------------------------------------------

void
string::
clear() noexcept
{
    s_.term(0);
}

//------------------------------------------------------------------------------

void
string::
push_back(char ch)
{
    *s_.append(1, sp_) = ch;
}

void
string::
pop_back()
{
    s_.data()[--s_.size] = 0;
}

//------------------------------------------------------------------------------

string&
string::
append(size_type count, char ch)
{
    traits_type::assign(
        s_.append(count, sp_),
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
        s_.append(count, sp_),
        s, count);
    return *this;
}

//------------------------------------------------------------------------------

string&
string::
insert(
    size_type pos,
    size_type count,
    char ch)
{
    traits_type::assign(
        s_.insert(pos, count, sp_),
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
    if(pos >= s_.size)
        BOOST_THROW_EXCEPTION(
            std::out_of_range(
                "pos >= size()"));
    if(count > s_.capacity - s_.size)
    {
        traits_type::copy(
            s_.insert(pos, count, sp_),
            s, count);
        return *this;
    }
    // VFALCO TODO handle [s, s+count) inside *this
    traits_type::move(
        s_.data() + pos + count,
        s_.data() + pos,
        s_.size - pos + 1);
    traits_type::copy(
        s_.data() + pos,
        s, count);
    s_.size += static_cast<
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


//------------------------------------------------------------------------------

string&
string::
erase(
    size_type pos,
    size_type count)
{
    if(pos > s_.size)
        BOOST_THROW_EXCEPTION(
            std::out_of_range(
                "pos > size()"));
    if( count > s_.size - pos)
        count = s_.size - pos;
    traits_type::move(
        s_.data() + pos,
        s_.data() + pos + count,
        s_.size - pos - count + 1);
    s_.size -= static_cast<
        impl_size_type>(count);
    s_.data()[s_.size] = 0;
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

//------------------------------------------------------------------------------

void
string::
resize(size_type count, char ch)
{
    if(count <= s_.size)
    {
        auto const n = static_cast<
            impl_size_type>(count);
        s_.data()[n] = 0;
        s_.size = n;
        return;
    }

    reserve(count);
    traits_type::assign(
        s_.end(),
        count + 1 - s_.size,
        ch);
    s_.size = static_cast<
        impl_size_type>(count);
}

//------------------------------------------------------------------------------

void
string::
swap(string& other)
{
    if(*sp_ == *other.sp_)
    {
        std::swap(s_, other.s_);
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

//------------------------------------------------------------------------------

std::ostream&
operator<<(std::ostream& os, string const& s)
{
    os.write(s.data(), s.size());
    return os;
}

} // json
} // boost

#endif
