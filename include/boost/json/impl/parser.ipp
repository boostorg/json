//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_PARSER_IPP
#define BOOST_JSON_IMPL_PARSER_IPP

#include <boost/json/parser.hpp>
#include <boost/json/error.hpp>
#include <boost/json/detail/assert.hpp>
#include <boost/json/detail/except.hpp>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace boost {
namespace json {

//----------------------------------------------------------

/*

Stack Layout:
    ... denotes 0 or more
    <> denotes empty storage

array
    saved_state
    std::size_t
    state
    value...
    <value>

object
    saved_state
    std::size_t
    state   
    value_type...
    <value_type>

key
    (chars)...
    std::size_t
*/

enum class parser::state : char
{
    need_start, // start() not called yet
    begin,      // we have a storage_ptr

    // These states indicate what is
    // currently at top of the stack.

    top,        // empty top value
    arr,        // empty array value
    obj,        // empty object value
    key,        // complete key
    end         // compelte top value
};

void
parser::
destroy() noexcept
{
    if(key_size_ > 0)
    {
        // remove partial key
        BOOST_JSON_ASSERT(
            st_ == state::obj);
        BOOST_JSON_ASSERT(
            str_size_ == 0);
        rs_.subtract(key_size_);
        key_size_ = 0;
    }
    else if(str_size_ > 0)
    {
        // remove partial string
        rs_.subtract(str_size_);
        str_size_ = 0;
    }
    // unwind the rest
    do
    {
        switch(st_)
        {
        case state::need_start:
            BOOST_JSON_ASSERT(
                rs_.empty());
            break;

        case state::begin:
            BOOST_JSON_ASSERT(
                rs_.empty());
            break;

        case state::top:
            rs_.subtract(
                sizeof(value));
            BOOST_JSON_ASSERT(
                rs_.empty());
            break;

        case state::arr:
        {
            pop_array();
            pop(st_);
            pop(count_);
            saved_state ss;
            pop(ss);
            break;
        }

        case state::obj:
        {
            pop_object();
            pop(st_);
            pop(count_);
            saved_state ss;
            pop(ss);
            break;
        }

        case state::key:
        {
            std::size_t key_size;
            pop(key_size);
            pop_chars(key_size);
            st_ = state::obj;
            break;
        }

        case state::end:
        {
            auto ua =
                pop_array();
            BOOST_JSON_ASSERT(
                ua.size() == 1);
            BOOST_JSON_ASSERT(
                rs_.empty());
            break;
        }

        }
    }
    while(! rs_.empty());
}

parser::
~parser()
{
    destroy();
}

parser::
parser()
    : st_(state::need_start)
{
}

void
parser::
start(storage_ptr sp) noexcept
{
    clear();
    sp_ = std::move(sp);
    st_ = state::begin;
}

void
parser::
clear() noexcept
{
    destroy();
    rs_.clear();
    basic_parser::reset();
    count_ = 0;
    key_size_ = 0;
    str_size_ = 0;
    st_ = state::need_start;
    sp_ = {};
}

value
parser::
release() noexcept
{
    if(is_done())
    {
        BOOST_JSON_ASSERT(st_ == state::end);
        auto ua = pop_array();
        BOOST_JSON_ASSERT(rs_.empty());
        union U
        {
            value v;
            U(){}
            ~U(){}
        };
        U u;
        ua.relocate(&u.v);
        basic_parser::reset();
        st_ = state::need_start;
        sp_ = {};
        return std::move(u.v);
    }
    // return null
    value jv(std::move(sp_));
    clear();
    return jv;
}

//----------------------------------------------------------

template<class T>
void
parser::
push(T const& t)
{
    std::memcpy(
        rs_.push(sizeof(T)),
        &t, sizeof(T));
}

void
parser::
push_chars(string_view s)
{
    std::memcpy(
        rs_.push(s.size()),
        s.data(), s.size());
}

template<class... Args>
void
parser::
emplace(Args&&... args)
{
    if(st_ == state::key)
    {
        union U
        {
            object::value_type v;
            U(){}
            ~U(){}
        };
        U u;
        // perform stack reallocation up-front
        // VFALCO This is more than we need
        rs_.prepare(sizeof(u.v));
        std::size_t key_size;
        pop(key_size);
        auto const key =
            pop_chars(key_size);
        st_ = state::obj;
        ::new(&u.v) object::value_type(
            key, std::forward<Args>(args)...);
        rs_.subtract(sizeof(u.v));
        push(u.v);
        rs_.add(sizeof(u.v));
    }
    else
    {
        BOOST_JSON_ASSERT(
            st_ == state::arr ||
            st_ == state::top);
        union U
        {
            value v;
            U(){}
            ~U(){}
        };
        U u;
        // prevent splits from exceptions
        rs_.prepare(2 * sizeof(value));
        ::new(&u.v) value(
            std::forward<Args>(args)...);
        rs_.subtract(sizeof(u.v));
        push(u.v);
        rs_.add(sizeof(u.v));
    }
    ++count_;
}

template<class T>
void
parser::
pop(T& t)
{
    std::memcpy(&t,
        rs_.pop(sizeof(T)),
        sizeof(T));
}

unchecked_object
parser::
pop_object() noexcept
{
    rs_.subtract(sizeof(
        object::value_type));
    if(count_ == 0)
        return { nullptr, 0, sp_ };
    auto const n = count_ * sizeof(
        object::value_type);
    return { reinterpret_cast<
        object::value_type*>(rs_.pop(n)),
        count_, sp_ };
}

unchecked_array
parser::
pop_array() noexcept
{
    rs_.subtract(sizeof(value));
    if(count_ == 0)
        return { nullptr, 0, sp_ };
    auto const n =
        count_ * sizeof(value);
    return { reinterpret_cast<value*>(
        rs_.pop(n)), count_, sp_ };
}

string_view
parser::
pop_chars(
    std::size_t size) noexcept
{
    return {
        reinterpret_cast<char const*>(
            rs_.pop(size)), size };
}

//----------------------------------------------------------

void
parser::
on_document_begin(
    error_code& ec)
{
    if(st_ == state::need_start)
    {
        ec = error::need_start;
        return;
    }

    count_ = 0;
    key_size_ = 0;
    str_size_ = 0;

    // The top level `value` is kept
    // inside a notional 1-element array.
    rs_.add(sizeof(value));
    st_ = state::top;
}

void
parser::
on_document_end(error_code&)
{
    BOOST_JSON_ASSERT(count_ == 1);
    st_ = state::end; // VFALCO RECONSIDER
}

void
parser::
on_object_begin(error_code&)
{
    // prevent splits from exceptions
    rs_.prepare(
        sizeof(saved_state) +
        sizeof(count_) +
        sizeof(st_) +
        sizeof(object::value_type));
    push(save_state());
    push(count_);
    push(st_);
    rs_.add(sizeof(
        object::value_type));

    count_ = 0;
    st_ = state::obj;
}

void
parser::
on_object_end(error_code&)
{
    BOOST_JSON_ASSERT(
        st_ == state::obj);
    auto uo = pop_object();
    pop(st_);
    pop(count_);
    saved_state ss;
    pop(ss);
    restore_state(ss);
    emplace(std::move(uo));
}

void
parser::
on_array_begin(error_code&)
{
    // prevent splits from exceptions
    rs_.prepare(
        sizeof(saved_state) +
        sizeof(count_) +
        sizeof(st_) +
        sizeof(value));
    push(save_state());
    push(count_);
    push(st_);
    rs_.add(sizeof(value));

    count_ = 0;
    st_ = state::arr;
}

void
parser::
on_array_end(error_code&)
{
    BOOST_JSON_ASSERT(
        st_ == state::arr);
    auto ua = pop_array();
    pop(st_);
    pop(count_);
    saved_state ss;
    pop(ss);
    restore_state(ss);
    emplace(std::move(ua));
}

void
parser::
on_key_part(
    string_view s,
    error_code&)
{
    if( s.size() >
        string::max_size() - key_size_)
        BOOST_JSON_THROW(
            detail::key_too_large_exception());  
    push_chars(s);
    key_size_ += static_cast<size_type>(
        s.size());
}

void
parser::
on_key(
    string_view s,
    error_code& ec)
{
    BOOST_JSON_ASSERT(
        st_ == state::obj);
    on_key_part(s, ec);
    push(key_size_);
    key_size_ = 0;
    st_ = state::key;
}

void
parser::
on_string_part(
    string_view s,
    error_code&)
{
    if( s.size() >
        string::max_size() - str_size_)
        BOOST_JSON_THROW(
            detail::string_too_large_exception());  
    push_chars(s);
    str_size_ += static_cast<size_type>(
        s.size());
}

void
parser::
on_string(
    string_view s,
    error_code&)
{
    if( s.size() >
        string::max_size() - str_size_)
        BOOST_JSON_THROW(
            detail::string_too_large_exception());  
    if(str_size_ == 0)
    {
        // fast path
        emplace(s, sp_);
    }
    else
    {
        string str(sp_);
        auto const sv =
            pop_chars(str_size_);
        str_size_ = 0;
        str.reserve(
            sv.size() + s.size());
        std::memcpy(
            str.data(),
            sv.data(), sv.size());
        std::memcpy(
            str.data() + sv.size(),
            s.data(), s.size());
        str.grow(sv.size() + s.size());
        emplace(std::move(str));
    }
}

void
parser::
on_int64(
    int64_t i,
    error_code&)
{
    emplace(i, sp_);
}

void
parser::
on_uint64(
    uint64_t u,
    error_code&)
{
    emplace(u, sp_);
}

void
parser::
on_double(
    double d,
    error_code&)
{
    emplace(d, sp_);
}

void
parser::
on_bool(bool b, error_code&)
{
    emplace(b, sp_);
}

void
parser::
on_null(error_code&)
{
    emplace(nullptr, sp_);
}

//----------------------------------------------------------

value
parse(
    string_view s,
    storage_ptr sp,
    error_code& ec)
{
    parser p;
    p.start(std::move(sp));
    p.write(s.data(), s.size(), ec);
    return p.release();
}

value
parse(
    string_view s,
    storage_ptr sp)
{
    error_code ec;
    auto jv = parse(s, std::move(sp), ec);
    if(ec)
        BOOST_JSON_THROW(
            system_error(ec));
    return jv;
}

} // json
} // boost

#endif
