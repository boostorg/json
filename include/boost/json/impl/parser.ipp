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

void
parser::
on_document_begin(error_code&)
{
}

void
parser::
on_document_end(error_code&)
{
    BOOST_JSON_ASSERT(size_ == 1);
    auto ua = st_.pop_array(1);
    jv_.~value();
    ua.relocate(&jv_);
}

void
parser::
on_object_begin(error_code&)
{
    st_.emplace_value(object_kind);
    st_.push(size_ + 1);
    size_ = 0;
}

void
parser::
on_object_end(error_code&)
{
    auto ua = st_.pop_array(size_);
    st_.pop(size_);
}

void
parser::
on_array_begin(error_code&)
{
    st_.emplace_value(array_kind);
    st_.push(size_ + 1);
    size_ = 0;
}

void
parser::
on_array_end(error_code&)
{
    auto ua = st_.pop_array(size_);
    st_.pop(size_);
    st_.top_value().get_array().assign(std::move(ua));
}

void
parser::
on_key_part(
    string_view,
    error_code&)
{
}

void
parser::
on_key(
    string_view,
    error_code&)
{
}

void
parser::
on_string_part(
    string_view s,
    error_code&)
{
    if( s.size() >
        detail::max_string_length_ - len_)
        BOOST_JSON_THROW(
            detail::string_too_large_exception());  
    st_.push(s);
    len_ += static_cast<size_type>(
        s.size());
}

void
parser::
on_string(
    string_view s,
    error_code&)
{
    if( s.size() >
        detail::max_string_length_ - len_)
        BOOST_JSON_THROW(
            detail::string_too_large_exception());  
    if(len_ == 0)
    {
        // fast path
        st_.emplace_value(s);
    }
    else
    {
        auto& js = st_.emplace_value(
            string_kind).get_string();
        js.reserve(len_ + s.size());
        auto const ss = st_.pop_string(len_);
        std::memcpy(
            js.data(), ss.data(), ss.size());
        std::memcpy(
            js.data() + ss.size(),
            s.data(), s.size());
        js.grow(ss.size() + s.size());
    }
    len_ = 0;
    ++size_;
}

void
parser::
on_int64(
    int64_t i,
    error_code&)
{
    ++size_;
    st_.emplace_value(i);
}

void
parser::
on_uint64(
    uint64_t u,
    error_code&)
{
    ++size_;
    st_.emplace_value(u);
}

void
parser::
on_double(
    double d,
    error_code&)
{
    ++size_;
    st_.emplace_value(d);
}

void
parser::
on_bool(bool b, error_code&)
{
    ++size_;
    st_.emplace_value(b);
}

void
parser::
on_null(error_code&)
{
    ++size_;
    st_.emplace_value(nullptr);
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
