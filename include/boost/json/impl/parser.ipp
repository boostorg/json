//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_PARSER_IPP
#define BOOST_JSON_IMPL_PARSER_IPP

#include <boost/json/parser.hpp>
#include <boost/json/error.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace json {

parser::
parser()
{
}

parser::
parser(storage_ptr const& store)
    : jv_(store)
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

void
parser::
on_document_begin(error_code&)
{
    stack_.clear();
    stack_.push_front(&jv_);
    s_.clear();
    obj_ = false;
}

void
parser::
on_object_begin(error_code& ec)
{
    if(stack_.size() >= max_depth_)
    {
        ec = error::too_deep;
        return;
    }
    auto& jv = *stack_.front();
    BOOST_ASSERT(! jv.is_object());
    if(obj_)
    {
        BOOST_ASSERT(jv.is_null());
        jv.emplace_object();
    }
    else if(jv.is_array())
    {
        jv.as_array().emplace_back(
            kind::object);
        stack_.push_front(
            &jv.as_array().back());
    }
    else
    {
        BOOST_ASSERT(jv.is_null());
        jv = kind::object;
    }
    obj_ = true;
}

void
parser::
on_object_end(error_code&)
{
    BOOST_ASSERT(
        stack_.front()->is_object());
    stack_.pop_front();
    if(! stack_.empty())
    {
        auto const& jv = stack_.front();
        BOOST_ASSERT(
            jv->is_array() || jv->is_object());
        obj_ = jv->is_object();
    }
}

void
parser::
on_array_begin(error_code& ec)
{
    if(stack_.size() >= max_depth_)
    {
        ec = error::too_deep;
        return;
    }
    auto& jv = *stack_.front();
    BOOST_ASSERT(! jv.is_object());
    if(obj_)
    {
        BOOST_ASSERT(jv.is_null());
        jv.emplace_array();
    }
    else if(jv.is_array())
    {
        BOOST_ASSERT(s_.empty());
        jv.as_array().emplace_back(
            kind::array);
        stack_.push_front(
            &jv.as_array().back());
    }
    else
    {
        BOOST_ASSERT(jv.is_null());
        jv = kind::array;
    }
    obj_ = false;
}

void
parser::
on_array_end(error_code&)
{
    BOOST_ASSERT(
        stack_.front()->is_array());
    stack_.pop_front();
    if(! stack_.empty())
    {
        auto const& jv = stack_.front();
        BOOST_ASSERT(
            jv->is_array() || jv->is_object());
        obj_ = jv->is_object();
    }
}

void
parser::
on_key_data(
    string_view s,
    error_code&)
{
    s_.append(s.data(), s.size());
}

void
parser::
on_key_end(
    string_view s,
    error_code&)
{
    auto& jv = *stack_.front();
    if(! s_.empty())
    {
        s_.append(s.data(), s.size());
        s = {s_.data(), s_.size()};
    }
    if(jv.is_object())
    {
        stack_.push_front(
            &jv.as_object().emplace(s,
                kind::null).first->second);
    }
    else if(stack_.front()->is_array())
    {
        BOOST_ASSERT(s_.empty());
        jv.as_array().emplace_back(kind::null);
    }
    else
    {
        BOOST_ASSERT(jv.is_null());
        // nothing to do
    }
    s_.clear();
}

void
parser::
on_string_data(
    string_view s, error_code&)
{
    auto& jv = *stack_.front();
    BOOST_ASSERT(! jv.is_object());
    if(! jv.is_string())
    {
        if(obj_)
        {
            BOOST_ASSERT(jv.is_null());
            jv.emplace_string().append(
                s.data(), s.size());
        }
        else if(stack_.front()->is_array())
        {
            BOOST_ASSERT(s_.empty());
            jv.as_array().emplace_back(kind::string);
            stack_.push_front(
                &jv.as_array().back());
            stack_.front()->as_string().append(
                s.data(), s.size());
        }
        else
        {
            BOOST_ASSERT(jv.is_null());
            jv.emplace_string().append(
                s.data(), s.size());
        }
    }
    else
    {
        stack_.front()->as_string().append(
            s.data(), s.size());
    }
}

void
parser::
on_string_end(
    string_view s,
    error_code& ec)
{
    on_string_data(s, ec);
    BOOST_ASSERT(stack_.front()->is_string());
    stack_.pop_front();
    if(! stack_.empty())
    {
        auto const& jv = stack_.front();
        BOOST_ASSERT(
            jv->is_array() || jv->is_object());
        obj_ = jv->is_object();
    }
}

void
parser::
on_number(number n, error_code&)
{
    assign(n);
}

void
parser::
on_bool(bool b, error_code&)
{
    assign(b);
}

void
parser::
on_null(error_code&)
{
    assign(null);
}

} // json
} // boost

#endif
