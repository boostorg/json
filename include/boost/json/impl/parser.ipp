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
    : st_(sp)
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

void
parser::
destroy()
{
    if(key_ > 0)
    {
        BOOST_JSON_ASSERT(lev_.obj);
        BOOST_JSON_ASSERT(str_ == 0);
        st_.unreserve(key_);
    }
    if(str_ > 0)
    {
        st_.unreserve(str_);
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
}

void
parser::
on_document_begin(error_code&)
{
    lev_ = level{ 0, false };
    st_.placeholder(sizeof(value));
}

void
parser::
on_document_end(error_code&)
{
    BOOST_JSON_ASSERT(lev_.size == 1);
    st_.unreserve(sizeof(value));
    auto ua = st_.pop_array(1);
    jv_.~value();
    ua.relocate(&jv_);
}

void
parser::
on_object_begin(error_code&)
{
    st_.push(lev_);
    lev_ = level{ 0, true };
    st_.placeholder(sizeof(
        object::value_type));
}

void
parser::
on_object_end(error_code&)
{
    st_.unreserve(sizeof(
        object::value_type));
    auto uo = st_.pop_object(lev_.size);
    st_.pop(lev_);
    if(lev_.obj)
    {
        st_.emplace_pair(
            std::move(uo));
        st_.placeholder(
            sizeof(object::value_type));
    }
    else
    {
        st_.emplace_value(std::move(uo));
        st_.placeholder(sizeof(value));
    }
    ++lev_.size;
}

void
parser::
on_array_begin(error_code&)
{
    st_.push(lev_);
    lev_ = level{ 0, false };
    st_.placeholder(sizeof(value));
}

void
parser::
on_array_end(error_code&)
{
    st_.unreserve(sizeof(value));
    auto ua = st_.pop_array(lev_.size);
    st_.pop(lev_);
    if(lev_.obj)
    {
        st_.emplace_pair(
            std::move(ua));
        st_.placeholder(
            sizeof(object::value_type));
    }
    else
    {
        st_.emplace_value(std::move(ua));
        st_.placeholder(sizeof(value));
    }
    ++lev_.size;
}

void
parser::
on_key_part(
    string_view s,
    error_code&)
{
    if( s.size() >
        detail::max_string_length_ - key_)
        BOOST_JSON_THROW(
            detail::key_too_large_exception());  
    st_.push(s);
    key_ += static_cast<size_type>(
        s.size());
}

void
parser::
on_key(
    string_view s,
    error_code&)
{
    if( s.size() >
        detail::max_string_length_ - key_)
        BOOST_JSON_THROW(
            detail::key_too_large_exception());  
    st_.push(s);
    key_ += static_cast<size_type>(
        s.size());
    st_.align();
    st_.push(key_);
    key_ = 0;
}

void
parser::
on_string_part(
    string_view s,
    error_code&)
{
    if( s.size() >
        detail::max_string_length_ - str_)
        BOOST_JSON_THROW(
            detail::string_too_large_exception());  
    st_.push(s);
    str_ += static_cast<size_type>(
        s.size());
}

void
parser::
on_string(
    string_view s,
    error_code&)
{
    if( s.size() >
        detail::max_string_length_ - str_)
        BOOST_JSON_THROW(
            detail::string_too_large_exception());  
    if(str_ == 0)
    {
        // fast path
        if(lev_.obj)
        {
            st_.emplace_pair(s);
            st_.placeholder(
                sizeof(object::value_type));
        }
        else
        {
            st_.emplace_value(s);
            st_.placeholder(sizeof(value));
        }
        ++lev_.size;
    }
    else
    {
        auto const ss = st_.pop_string(str_);
        string* const ps = [&]
        {
            if(lev_.obj)
                return &st_.emplace_pair(
                    string_kind).value().get_string();
            return &st_.emplace_value(
                string_kind).get_string();
        }();
        if(lev_.obj)
            st_.placeholder(
                sizeof(object::value_type));
        else
            st_.placeholder(sizeof(value));
        ++lev_.size;
        ps->reserve(str_ + s.size());
        std::memcpy(ps->data(),
            ss.data(), ss.size());
        std::memcpy(ps->data() + ss.size(),
            s.data(), s.size());
        ps->grow(ss.size() + s.size());
    }
    str_ = 0;
}

void
parser::
on_int64(
    int64_t i,
    error_code&)
{
    if(lev_.obj)
    {
        st_.emplace_pair(i);
        st_.placeholder(
            sizeof(object::value_type));
    }
    else
    {
        st_.emplace_value(i);
        st_.placeholder(sizeof(value));
    }
    ++lev_.size;
}

void
parser::
on_uint64(
    uint64_t u,
    error_code&)
{
    if(lev_.obj)
    {
        st_.emplace_pair(u);
        st_.placeholder(
            sizeof(object::value_type));
    }
    else
    {
        st_.emplace_value(u);
        st_.placeholder(sizeof(value));
    }
    ++lev_.size;
}

void
parser::
on_double(
    double d,
    error_code&)
{
    if(lev_.obj)
    {
        st_.emplace_pair(d);
        st_.placeholder(
            sizeof(object::value_type));
    }
    else
    {
        st_.emplace_value(d);
        st_.placeholder(sizeof(value));
    }
    ++lev_.size;
}

void
parser::
on_bool(bool b, error_code&)
{
    if(lev_.obj)
    {
        st_.emplace_pair(b);
        st_.placeholder(
            sizeof(object::value_type));
    }
    else
    {
        st_.emplace_value(b);
        st_.placeholder(sizeof(value));
    }
    ++lev_.size;
}

void
parser::
on_null(error_code&)
{
    if(lev_.obj)
    {
        st_.emplace_pair(nullptr);
        st_.placeholder(
            sizeof(object::value_type));
    }
    else
    {
        st_.emplace_value(nullptr);
        st_.placeholder(sizeof(value));
    }
    ++lev_.size;
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
