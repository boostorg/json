//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
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
    end         // complete top value
};

void
parser::
destroy() noexcept
{
    if(key_size_ > 0)
    {
        // remove partial key
        BOOST_ASSERT(
            lev_.st == state::obj);
        BOOST_ASSERT(
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
        switch(lev_.st)
        {
        case state::need_start:
            BOOST_ASSERT(
                rs_.empty());
            break;

        case state::begin:
            BOOST_ASSERT(
                rs_.empty());
            break;

        case state::top:
            rs_.subtract(
                sizeof(value));
            BOOST_ASSERT(
                rs_.empty());
            break;

        case state::arr:
        {
            pop_array();
            rs_.subtract(lev_.align);
            pop(lev_);
            break;
        }

        case state::obj:
        {
            pop_object();
            rs_.subtract(lev_.align);
            pop(lev_);
            break;
        }

        case state::key:
        {
            std::uint32_t key_size;
            pop(key_size);
            pop_chars(key_size);
            lev_.st = state::obj;
            break;
        }

        case state::end:
        {
            auto ua =
                pop_array();
            BOOST_ASSERT(
                ua.size() == 1);
            BOOST_ASSERT(
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
{
    lev_.st = state::need_start;
}

void
parser::
reserve(
    std::size_t bytes) noexcept
{
    rs_.reserve(bytes);
}

void
parser::
start(storage_ptr sp) noexcept
{
    clear();
    sp_ = std::move(sp);
    lev_.st = state::begin;
}

void
parser::
clear() noexcept
{
    destroy();
    rs_.clear();
    basic_parser::reset();
    lev_.count = 0;
    key_size_ = 0;
    str_size_ = 0;
    lev_.st = state::need_start;
    sp_ = {};
}

value
parser::
release()
{
    if(! is_done())
        BOOST_THROW_EXCEPTION(
            std::logic_error(
                "no value"));
    BOOST_ASSERT(lev_.st == state::end);
    auto ua = pop_array();
    BOOST_ASSERT(rs_.empty());
    union U
    {
        value v;
        U(){}
        ~U(){}
    };
    U u;
    ua.relocate(&u.v);
    basic_parser::reset();
    lev_.st = state::need_start;
    sp_ = {};
    return pilfer(u.v);
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
    if(lev_.st == state::key)
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
        rs_.prepare(sizeof(object::value_type));
        std::uint32_t key_size;
        pop(key_size);
        auto const key =
            pop_chars(key_size);
        lev_.st = state::obj;
        BOOST_ASSERT((rs_.top() %
            alignof(object::value_type)) == 0);
        ::new(rs_.behind(
            sizeof(object::value_type)))
                object::value_type(
            key, std::forward<Args>(args)...);
        rs_.add(sizeof(u.v));
    }
    else if(lev_.st == state::arr)
    {
        // prevent splits from exceptions
        rs_.prepare(sizeof(value));
        BOOST_ASSERT((rs_.top() %
            alignof(value)) == 0);
        ::new(rs_.behind(sizeof(value))) value(
            std::forward<Args>(args)...);
        rs_.add(sizeof(value));
    }
    else
    {
        BOOST_ASSERT(
            lev_.st == state::top);
        // prevent splits from exceptions
        rs_.prepare(sizeof(value));
        BOOST_ASSERT((rs_.top() %
            alignof(value)) == 0);
        ::new(rs_.behind(sizeof(value))) value(
            std::forward<Args>(args)...);
        rs_.add(sizeof(value));
        lev_.st = state::end; // VFALCO Maybe pre_end
    }
    ++lev_.count;
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

detail::unchecked_object
parser::
pop_object() noexcept
{
    rs_.subtract(sizeof(
        object::value_type));
    if(lev_.count == 0)
        return { nullptr, 0, sp_ };
    auto const n = lev_.count * sizeof(
        object::value_type);
    return { reinterpret_cast<
        object::value_type*>(rs_.pop(n)),
        lev_.count, sp_ };
}

detail::unchecked_array
parser::
pop_array() noexcept
{
    rs_.subtract(sizeof(value));
    if(lev_.count == 0)
        return { nullptr, 0, sp_ };
    auto const n =
        lev_.count * sizeof(value);
    return { reinterpret_cast<value*>(
        rs_.pop(n)), lev_.count, sp_ };
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
    if(lev_.st == state::need_start)
    {
        ec = error::need_start;
        return;
    }

    lev_.count = 0;
    lev_.align = 0;
    key_size_ = 0;
    str_size_ = 0;

    // The top level `value` is kept
    // inside a notional 1-element array.
    rs_.add(sizeof(value));
    lev_.st = state::top;
}

void
parser::
on_document_end(error_code&)
{
    BOOST_ASSERT(lev_.count == 1);
}

void
parser::
on_object_begin(error_code&)
{
    // prevent splits from exceptions
    rs_.prepare(
        sizeof(level) +
        sizeof(object::value_type) +
        alignof(object::value_type) - 1);
    lev_.ss = save_state();
    push(lev_);
    lev_.align = detail::align_to<
        object::value_type>(rs_);
    rs_.add(sizeof(
        object::value_type));
    lev_.count = 0;
    lev_.st = state::obj;
}

void
parser::
on_object_end(error_code&)
{
    BOOST_ASSERT(
        lev_.st == state::obj);
    auto uo = pop_object();
    rs_.subtract(lev_.align);
    pop(lev_);
    restore_state(lev_.ss);
    emplace(std::move(uo));
}

void
parser::
on_array_begin(error_code&)
{
    // prevent splits from exceptions
    rs_.prepare(
        sizeof(level) +
        sizeof(value) +
        alignof(value) - 1);
    lev_.ss = save_state();
    push(lev_);
    lev_.align =
        detail::align_to<value>(rs_);
    rs_.add(sizeof(value));
    lev_.count = 0;
    lev_.st = state::arr;
}

void
parser::
on_array_end(error_code&)
{
    BOOST_ASSERT(
        lev_.st == state::arr);
    auto ua = pop_array();
    rs_.subtract(lev_.align);
    pop(lev_);
    restore_state(lev_.ss);
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
        BOOST_THROW_EXCEPTION(
            key_too_large());
    push_chars(s);
    key_size_ += static_cast<
        std::uint32_t>(s.size());
}

void
parser::
on_key(
    string_view s,
    error_code& ec)
{
    BOOST_ASSERT(
        lev_.st == state::obj);
    on_key_part(s, ec);
    push(key_size_);
    key_size_ = 0;
    lev_.st = state::key;
}

void
parser::
on_string_part(
    string_view s,
    error_code&)
{
    if( s.size() >
        string::max_size() - str_size_)
        BOOST_THROW_EXCEPTION(
            string_too_large());  
    push_chars(s);
    str_size_ += static_cast<
        std::uint32_t>(s.size());
}

void
parser::
on_string(
    string_view s,
    error_code&)
{
    if( s.size() >
        string::max_size() - str_size_)
        BOOST_THROW_EXCEPTION(
            string_too_large());  
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
    error_code& ec,
    storage_ptr sp)
{
    parser p;
    p.start(std::move(sp));
    p.finish(
        s.data(),
        s.size(),
        ec);
    if(ec)
        return nullptr;
    return p.release();
}

value
parse(
    string_view s,
    storage_ptr sp)
{
    error_code ec;
    auto jv = parse(
        s, ec, std::move(sp));
    if(ec)
        BOOST_THROW_EXCEPTION(
            system_error(ec));
    return jv;
}

} // json
} // boost

#endif
