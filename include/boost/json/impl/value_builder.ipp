//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_IMPL_VALUE_BUILDER_IPP
#define BOOST_JSON_IMPL_VALUE_BUILDER_IPP

#include <boost/json/value_builder.hpp>
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

enum class value_builder::state : char
{
    need_reset, // reset() not called yet
    begin,      // we have a storage_ptr

    // These states indicate what is
    // currently at top of the stack.

    top,        // top value, constructed if lev_.count==1
    arr,        // empty array value
    obj,        // empty object value
    key,        // complete key
};

value_builder::
~value_builder()
{
    destroy();
}

value_builder::
value_builder(
    storage_ptr sp) noexcept
    : rs_(std::move(sp))
{
    lev_.st = state::need_reset;
}

void
value_builder::
reserve(std::size_t n)
{
#ifndef BOOST_NO_EXCEPTIONS
    try
    {
#endif
        rs_.reserve(n);
#ifndef BOOST_NO_EXCEPTIONS
    }
    catch(std::bad_alloc const&)
    {
        // squelch the exception, per contract
    }
#endif
}

void
value_builder::
reset(storage_ptr sp) noexcept
{
    clear();
    sp_ = std::move(sp);
    lev_.st = state::begin;
}

value
value_builder::
release()
{
    // An exception here means that the value
    // was not properly constructed. For example,
    // an array or object was not closed, or
    // there was no top level value.
    if( lev_.st != state::top ||
        lev_.count != 1)
        BOOST_THROW_EXCEPTION(
            std::logic_error(
                "no value"));
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
    lev_.st = state::need_reset;
    sp_ = {};
    return pilfer(u.v);
}

void
value_builder::
clear() noexcept
{
    destroy();
    rs_.clear();
    lev_.count = 0;
    key_size_ = 0;
    str_size_ = 0;
    lev_.st = state::need_reset;
    sp_ = {};
}

//----------------------------------------------------------

bool
value_builder::
on_document_begin(
    error_code&)
{
    // reset() must be called before
    // building every new top level value.
    BOOST_ASSERT(lev_.st == state::begin);

    lev_.count = 0;
    lev_.align = 0;
    key_size_ = 0;
    str_size_ = 0;

    // The top level `value` is kept
    // inside a notional 1-element array.
    rs_.add(sizeof(value));
    lev_.st = state::top;

    return true;
}

bool
value_builder::
on_document_end(error_code&)
{
    // If this goes off, then an
    // array or object was never finished.
    BOOST_ASSERT(lev_.st == state::top);
    BOOST_ASSERT(lev_.count == 1);
    return true;
}

bool
value_builder::
on_object_begin(error_code&)
{
    // prevent splits from exceptions
    rs_.prepare(
        sizeof(level) +
        sizeof(object::value_type) +
        alignof(object::value_type) - 1);
    push(lev_);
    lev_.align = detail::align_to<
        object::value_type>(rs_);
    rs_.add(sizeof(
        object::value_type));
    lev_.count = 0;
    lev_.st = state::obj;
    return true;
}

bool
value_builder::
on_object_end(
    error_code& ec)
{
    BOOST_ASSERT(
        lev_.st == state::obj);
    auto uo = pop_object();
    rs_.subtract(lev_.align);
    pop(lev_);
    return emplace(
        ec, std::move(uo));
}

bool
value_builder::
on_array_begin(error_code&)
{
    // prevent splits from exceptions
    rs_.prepare(
        sizeof(level) +
        sizeof(value) +
        alignof(value) - 1);
    push(lev_);
    lev_.align =
        detail::align_to<value>(rs_);
    rs_.add(sizeof(value));
    lev_.count = 0;
    lev_.st = state::arr;
    return true;
}

bool
value_builder::
on_array_end(
    error_code& ec)
{
    BOOST_ASSERT(
        lev_.st == state::arr);
    auto ua = pop_array();
    rs_.subtract(lev_.align);
    pop(lev_);
    return emplace(
        ec, std::move(ua));
}

bool
value_builder::
on_key_part(
    string_view s,
    error_code& ec)
{
    if( s.size() >
        string::max_size() - key_size_)
    {
        ec = error::key_too_large;
        return false;
    }
    push_chars(s);
    key_size_ += static_cast<
        std::uint32_t>(s.size());
    return true;
}

bool
value_builder::
on_key(
    string_view s,
    error_code& ec)
{
    BOOST_ASSERT(
        lev_.st == state::obj);
    if(! on_key_part(s, ec))
        return false;
    push(key_size_);
    key_size_ = 0;
    lev_.st = state::key;
    return true;
}

bool
value_builder::
on_string_part(
    string_view s,
    error_code& ec)
{
    if( s.size() >
        string::max_size() - str_size_)
    {
        ec = error::string_too_large;
        return false;
    }
    push_chars(s);
    str_size_ += static_cast<
        std::uint32_t>(s.size());
    return true;
}

bool
value_builder::
on_string(
    string_view s,
    error_code& ec)
{
    if( s.size() >
        string::max_size() - str_size_)
    {
        ec = error::string_too_large;
        return false;
    }
    if(str_size_ == 0)
    {
        // fast path
        return emplace(ec, s, sp_);
    }

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
    return emplace(
        ec, std::move(str), sp_);
}

bool
value_builder::
on_int64(
    int64_t i,
    string_view,
    error_code& ec)
{
    return emplace(ec, i, sp_);
}

bool
value_builder::
on_uint64(
    uint64_t u,
    string_view,
    error_code& ec)
{
    return emplace(ec, u, sp_);
}

bool
value_builder::
on_double(
    double d,
    string_view,
    error_code& ec)
{
    return emplace(ec, d, sp_);
}

bool
value_builder::
on_bool(
    bool b,
    error_code& ec)
{
    return emplace(ec, b, sp_);
}

bool
value_builder::
on_null(error_code& ec)
{
    return emplace(ec, nullptr, sp_);
}

//----------------------------------------------------------

void
value_builder::
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
        case state::need_reset:
            BOOST_ASSERT(
                rs_.empty());
            break;

        case state::begin:
            BOOST_ASSERT(
                rs_.empty());
            break;

        case state::top:
            if(lev_.count > 0)
            {
                BOOST_ASSERT(
                    lev_.count == 1);
                auto ua =
                    pop_array();
                BOOST_ASSERT(
                    ua.size() == 1);
                BOOST_ASSERT(
                    rs_.empty());
            }
            else
            {
                // never parsed a value
                rs_.subtract(
                    sizeof(value));
                BOOST_ASSERT(
                    rs_.empty());
            }
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
        }
    }
    while(! rs_.empty());
}

template<class T>
void
value_builder::
push(T const& t)
{
    std::memcpy(
        rs_.push(sizeof(T)),
        &t, sizeof(T));
}

void
value_builder::
push_chars(string_view s)
{
    std::memcpy(
        rs_.push(s.size()),
        s.data(), s.size());
}

template<class... Args>
void
value_builder::
emplace_object(
    Args&&... args)
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
    auto const key = pop_chars(key_size);
    lev_.st = state::obj;
    BOOST_ASSERT((rs_.top() %
        alignof(object::value_type)) == 0);
    ::new(rs_.behind(
        sizeof(object::value_type)))
            object::value_type(
        key, std::forward<Args>(args)...);
    rs_.add_unchecked(sizeof(u.v));
    ++lev_.count;
}

template<class... Args>
void
value_builder::
emplace_array(Args&&... args)
{
    // prevent splits from exceptions
    rs_.prepare(sizeof(value));
    BOOST_ASSERT((rs_.top() %
        alignof(value)) == 0);
    ::new(rs_.behind(sizeof(value))) value(
        std::forward<Args>(args)...);
    rs_.add_unchecked(sizeof(value));
    ++lev_.count;
}

template<class... Args>
bool
value_builder::
emplace(
    error_code& ec,
    Args&&... args)
{
    if(lev_.st == state::key)
    {
        if(lev_.count <
            object::max_size())
        {
            emplace_object(std::forward<
                Args>(args)...);
            return true;
        }
        ec = error::object_too_large;
        return false;
    }
    if(lev_.count <
        array::max_size())
    {
        emplace_array(std::forward<
            Args>(args)...);
        return true;
    }
    ec = error::array_too_large;
    return false;
}

template<class T>
void
value_builder::
pop(T& t)
{
    std::memcpy(&t,
        rs_.pop(sizeof(T)),
        sizeof(T));
}

detail::unchecked_object
value_builder::
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
value_builder::
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
value_builder::
pop_chars(
    std::size_t size) noexcept
{
    return {
        reinterpret_cast<char const*>(
            rs_.pop(size)), size };
}

} // json
} // boost

#endif
