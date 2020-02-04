//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_STRING_IPP
#define BOOST_JSON_IMPL_STRING_IPP

#include <boost/json/error.hpp>
#include <algorithm>
#include <new>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace boost {
namespace json {

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
        ::new(&other.impl_) detail::string_impl();
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
    impl_.shrink_to_fit(sp_);
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
    back() = 0;
    impl_.size(impl_.size() - 1);
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
append(string_view sv)
{
    traits_type::copy(
        impl_.append(sv.size(), sp_),
        sv.data(), sv.size());
    return *this;
}

//----------------------------------------------------------

string&
string::
insert(
    size_type pos,
    string_view sv)
{
    impl_.insert(pos, sv.data(), sv.size(), sp_);
    return *this;
}

string&
string::
insert(
    std::size_t pos,
    std::size_t count,
    char ch)
{
    traits_type::assign(
        impl_.insert_unchecked(pos, count, sp_),
        count, ch);
    return *this;
}

//----------------------------------------------------------

string&
string::
replace(
    std::size_t pos,
    std::size_t count,
    string_view sv)
{
    impl_.replace(pos, count, sv.data(), sv.size(), sp_);
    return *this;
}

string&
string::
replace(
    std::size_t pos,
    std::size_t count,
    std::size_t count2,
    char ch)
{
    traits_type::assign(
        impl_.replace_unchecked(pos, count, count2, sp_),
        count2, ch);
    return *this;
}

//----------------------------------------------------------

string&
string::
erase(
    size_type pos,
    size_type count)
{
    if(pos > impl_.size())
        char_pos_error::raise();
    if( count > impl_.size() - pos)
        count = impl_.size() - pos;
    traits_type::move(
        impl_.data() + pos,
        impl_.data() + pos + count,
        impl_.size() - pos - count + 1);
    impl_.term(impl_.size() - count);
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
    if(count <= impl_.size())
    {
        impl_.term(count);
        return;
    }

    reserve(count);
    traits_type::assign(
        impl_.end(),
        count - impl_.size(),
        ch);
    grow(count - size());
}

//----------------------------------------------------------

void
string::
swap(string& other)
{
    BOOST_ASSERT(this != &other);
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
    BOOST_ASSERT(
        new_cap >= impl_.capacity());
    if(new_cap > impl_.capacity())
    {
        // grow
        new_cap = detail::string_impl::growth(
            new_cap, impl_.capacity());
        detail::string_impl tmp(new_cap, sp_);
        traits_type::copy(tmp.data(),
            impl_.data(), impl_.size() + 1);
        tmp.size(impl_.size());
        impl_.destroy(sp_);
        impl_ = tmp;
        return;
    }
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
