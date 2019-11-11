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

void
parser::
destroy() noexcept
{
#if 0
    if(key_size_ > 0)
    {
        BOOST_JSON_ASSERT(lev_.obj);
        BOOST_JSON_ASSERT(str_size_ == 0);
        st_.unreserve(key_size_);
    }
    if(str_size_ > 0)
    {
        st_.unreserve(str_size_);
    }
    if(! st_.empty())
    {
        for(;;)
        {
            if(lev_.obj)
            {
                st_.unreserve(sizeof(
                    object::value_type));
                auto uo = st_.pop_object(
                    lev_.size);
            }
            else
            {
                st_.unreserve(sizeof(
                    value));
                auto ua = st_.pop_array(
                    lev_.size);
            }
            if(st_.empty())
                break;
            st_.pop(lev_);
        }
    }
#endif
}

parser::
~parser()
{
    destroy();
}

parser::
parser()
{
}

parser::
parser(storage_ptr sp)
    : sp_(std::move(sp))
{
}

value const&
parser::
get() const noexcept
{
    return jv_;
}

value
parser::
release() noexcept
{
    return std::move(jv_);
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
push_string(string_view s)
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
    if(obj_)
    {
        union U
        {
            object::value_type v;
            U(){}
            ~U(){}
        };
        U u;
        std::size_t key_size;
        pop(key_size);
        auto const key =
            pop_string(key_size);
        ::new(&u.v) object::value_type(
            key, std::forward<Args>(args)...);
        rs_.subtract(sizeof(u.v));
        push(u.v);
        rs_.add(sizeof(u.v));
    }
    else
    {
        union U
        {
            value v;
            U(){}
            ~U(){}
        };
        U u;
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
        // VFALCO bad narrowing
        static_cast<std::uint32_t>(count_),
        sp_ };
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
        rs_.pop(n)),
        // VFALCO bad narrowing
        static_cast<std::uint32_t>(count_),
        sp_ };
}

string_view
parser::
pop_string(
    std::size_t size) noexcept
{
    return {
        reinterpret_cast<char const*>(
            rs_.pop(size)), size };
}

//----------------------------------------------------------

void
parser::
on_document_begin(error_code&)
{
    count_ = 0;
    key_size_ = 0;
    str_size_ = 0;
    obj_ = false;
    rs_.add(sizeof(value));
}

void
parser::
on_document_end(error_code&)
{
    BOOST_JSON_ASSERT(count_ == 1);
    auto ua = pop_array();
    jv_.~value();
    ua.relocate(&jv_);
}

void
parser::
on_object_begin(error_code&)
{
    push(count_);
    push(obj_);
    count_ = 0;
    obj_ = true;
    rs_.add(sizeof(
        object::value_type));
}

void
parser::
on_object_end(error_code&)
{
    auto uo = pop_object();
    pop(obj_);
    pop(count_);
    emplace(std::move(uo));
}

void
parser::
on_array_begin(error_code&)
{
    push(count_);
    push(obj_);
    count_ = 0;
    obj_ = false;
    rs_.add(sizeof(value));
}

void
parser::
on_array_end(error_code&)
{
    auto ua = pop_array();
    pop(obj_);
    pop(count_);
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
    push_string(s);
    key_size_ += static_cast<size_type>(
        s.size());
}

void
parser::
on_key(
    string_view s,
    error_code& ec)
{
    on_key_part(s, ec);
    push(key_size_);
    key_size_ = 0;
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
    push_string(s);
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
            pop_string(str_size_);
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
    str_size_ = 0;
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
    parser p(std::move(sp));
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
