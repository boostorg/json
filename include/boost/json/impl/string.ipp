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
#include <stdexcept>
#include <utility>

namespace boost {
namespace json {

string::
~string()
{
    if(s_)
        impl::destroy(s_, sp_);
}

string::
string() noexcept
    : sp_(default_storage())
{
}

string::
string(storage_ptr sp) noexcept
    : sp_(std::move(sp))
{
}

string::
string(
    size_type count,
    char ch,
    storage_ptr sp)
    : sp_(std::move(sp))
{
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
    assign(other, pos, count);
}

string::
string(
    char const* s,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    assign(s);
}

string::
string(
    char const* s,
    size_type count,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    assign(s, count);
}

string::
string(string const& other)
    : sp_(default_storage())
{
    assign(other);
}

string::
string(
    string const& other,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    assign(other);
}

string::
string(pilfered<string> p) noexcept
    : s_(boost::exchange(
        p.get().s_, nullptr))
    , sp_(std::move(p.get().sp_))
{
}

string::
string(string&& other) noexcept
    : s_(boost::exchange(
        other.s_, nullptr))
    , sp_(other.sp_)
{
}

string::
string(
    string&& other,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    assign(std::move(other));
}

string::
string(
    std::initializer_list<char> init,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    assign(init);
}

string::
string(
    string_view sv,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    assign(sv);
}

string::
string(
    string_view sv,
    size_type pos,
    size_type n,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    assign(sv.substr(pos, n));
}

//------------------------------------------------------------------------------

string&
string::
operator=(string const& other)
{
    assign(other);
    return *this;
}

string&
string::
operator=(string&& other)
{
    assign(std::move(other));
    return *this;
}

string&
string::
operator=(char const* s)
{
    assign(s);
    return *this;
}

string&
string::
operator=(char ch)
{
    raw_resize(1);
    traits_type::assign(
        s_->data(), 1, ch);
    return *this;
}

string&
string::
operator=(
    std::initializer_list<char> init)
{
    assign(init);
    return *this;
}

string&
string::
operator=(string_view sv)
{
    assign(sv);
    return *this;
}

//------------------------------------------------------------------------------

string&
string::
assign(
    size_type count,
    char ch)
{
    raw_resize(count);
    traits_type::assign(
        s_->data(), count, ch);
    return *this;
}

string&
string::
assign(
    string const& other)
{
    raw_resize(other.size());
    traits_type::copy(data(),
        other.data(), other.size());
    return *this;
}

string&
string::
assign(
    string const& other,
    size_type pos,
    size_type count)
{
    return assign(
        other.substr(pos, count));
}

string&
string::
assign(string&& other)
{
    if(*sp_ != *other.sp_)
        return assign(other);

    auto s = s_;
    s_ = other.s_;
    other.s_ = nullptr;
    if(s)
        impl::destroy(s, sp_);
    return *this;
}

string&
string::
assign(
    char const* s,
    size_type count)
{
    raw_resize(count);
    traits_type::copy(
        data(), s, count);
    return *this;
}

string&
string::
assign(
    char const* s)
{
    return assign(s,
        traits_type::length(s));
}

string&
string::
assign(std::initializer_list<char> init)
{
    raw_resize(init.size());
    std::copy(
        init.begin(),
        init.end(),
        s_->data());
    return *this;
}

string&
string::
assign(string_view sv)
{
    return assign(
        sv.data(), sv.size());
}

string&
string::
assign(
    string_view sv,
    size_type pos,
    size_type count)
{
    return assign(
        sv.substr(pos, count));
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
    return s_->data()[pos];
}

char const&
string::
at(size_type pos) const
{
    if(pos >= size())
        BOOST_THROW_EXCEPTION(
            std::out_of_range(
                "pos >= size()"));
    return s_->data()[pos];
}

char&
string::
operator[](size_type pos)
{
    return s_->data()[pos];
}

const char&
string::
operator[](size_type pos) const
{
    return s_->data()[pos];
}

char&
string::
front()
{
    return s_->data()[0];
}

char const&
string::
front() const
{
    return s_->data()[0];
}

char&
string::
back()
{
    return s_->data()[s_->size - 1];
}

char const&
string::
back() const
{
    return s_->data()[s_->size - 1];
}

char*
string::
data() noexcept
{
    if(! s_)
        return const_cast<
            char*>("");
    return s_->data();
}

char const*
string::
data() const noexcept
{
    if(! s_)
        return "";
    return s_->data();
}

char const*
string::
c_str() const noexcept
{
    return data();
}

string::
operator string_view() const noexcept
{
    return {data(), size()};
}

//------------------------------------------------------------------------------
//
// Iterators
//
//------------------------------------------------------------------------------

auto
string::
begin() noexcept ->
    iterator
{
    if(! s_)
        return nullptr;
    return s_->data();
}

auto
string::
begin() const noexcept ->
    const_iterator
{
    if(! s_)
        return nullptr;
    return s_->data();
}

auto
string::
end() noexcept ->
    iterator
{
    if(! s_)
        return nullptr;
    return s_->end();
}

auto
string::
end() const noexcept ->
    const_iterator
{
    if(! s_)
        return nullptr;
    return s_->end();
}

auto
string::
rbegin() noexcept ->
    reverse_iterator
{
    if(! s_)
        return reverse_iterator(nullptr);
    return reverse_iterator(s_->end());
}

auto
string::
rbegin() const noexcept ->
    const_reverse_iterator
{
    if(! s_)
        return reverse_iterator(nullptr);
    return const_reverse_iterator(s_->end());
}

auto
string::
rend() noexcept ->
    reverse_iterator
{
    if(! s_)
        return reverse_iterator(nullptr);
    return reverse_iterator(s_->data());
}

auto
string::
rend() const noexcept ->
    const_reverse_iterator
{
    if(! s_)
        return reverse_iterator(nullptr);
    return const_reverse_iterator(s_->data());
}

//------------------------------------------------------------------------------
//
// Capacity
//
//------------------------------------------------------------------------------

bool
string::
empty() const noexcept
{
    if(! s_)
        return true;
    return s_->size == 0;
}

auto
string::
size() const noexcept ->
    size_type
{
    if(! s_)
        return 0;
    return s_->size;
}

void
string::
reserve(size_type new_capacity)
{
    if(new_capacity <= capacity())
        return;
    auto s = impl::construct(
        size(), new_capacity, sp_);
    traits_type::copy(
        s->data(), data(), size());
    impl::destroy(
        boost::exchange(s_, s), sp_);
}

auto
string::
capacity() const noexcept ->
    size_type
{
    if(! s_)
        return 0;
    return s_->capacity;
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
    if(s_)
        s_->term(0);
}

//------------------------------------------------------------------------------

void
string::
push_back(char ch)
{
    s_->data()[s_->size++] = ch;
    s_->data()[s_->size] = 0;
}

void
string::
pop_back()
{
    s_->data()[--s_->size] = 0;
}

//------------------------------------------------------------------------------

string&
string::
append(size_type count, char ch)
{
    traits_type::assign(
        raw_insert(size(), count),
        count, ch);
    return *this;
}

string&
string::
append(string const& str)
{
    return append(
        str.data(), str.size());
}

string&
string::
append(
    string const& str,
    size_type pos,
    size_type count)
{
    return append(str.substr(pos, count));
}

string&
string::
append(
    char const* s,
    size_type count)
{
    traits_type::copy(
        raw_insert(size(), count),
        s, count);
    return *this;
}

string&
string::
append(char const* s)
{
    return append(s,
        traits_type::length(s));
}

string&
string::
append(std::initializer_list<char> init)
{
    // TODO
    boost::ignore_unused(init);
    return *this;
}

string&
string::
append(string_view sv)
{
    return append(
        sv.data(), sv.size());
}

//------------------------------------------------------------------------------

string_view
string::
substr(
    size_type pos ,
    size_type count) const
{
    if(pos > size())
        BOOST_THROW_EXCEPTION(
            std::out_of_range("pos > size()"));
    if(! s_)
        return {};
    return {
        s_->data() + pos,
        std::min<size_type>(
            s_->size - pos,
            count)};
}

//------------------------------------------------------------------------------

void
string::
raw_resize(size_type n)
{
    if(n <= capacity())
    {
        if(s_)
            s_->term(n);
        return;
    }
    auto s = boost::exchange(s_,
        impl::construct(n, n, sp_));
    if(s)
        impl::destroy(s, sp_);
}

// insert `n` uninitialized chars at `pos`,
// reallocating as needed. does nothing if n==0.
// returns the insertion point
char*
string::
raw_insert(
    size_type pos,
    size_type n)
{
    if( n > max_size() ||
        size() > max_size() - n)
        BOOST_THROW_EXCEPTION(
            std::out_of_range(
                "size() + n > max_size()"));
    if(pos > size())
        BOOST_THROW_EXCEPTION(
            std::out_of_range(
                "pos > size()"));
    auto const new_size = size() + n;

    if(new_size <= capacity())
    {
        if(s_)
        {
            traits_type::move(
                s_->data() + pos + n,
                s_->data() + pos,
                s_->size + 1 - pos);
            s_->size = new_size;
        }
    }
    else
    {
        auto s = impl::construct(
            new_size, sp_);
        if(s_)
        {
            traits_type::copy(
                s->data(),
                s_->data(),
                pos);
            traits_type::copy(
                s->data() + pos + n,
                s_->data() + pos,
                s_->size + 1 - pos);
            s->size = new_size;
            impl::destroy(s_, sp_);
        }
        else
        {
            s->term(n);
        }
        s_ = s;
    }
    return s_->data() + pos;
}

std::ostream&
operator<<(std::ostream& os, string const& s)
{
    os.write(s.data(), s.size());
    return os;
}

} // json
} // boost

#endif
