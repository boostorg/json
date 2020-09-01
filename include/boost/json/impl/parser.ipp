//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_IMPL_PARSER_IPP
#define BOOST_JSON_IMPL_PARSER_IPP

#include <boost/json/basic_parser.hpp>
#include <boost/json/error.hpp>
#include <boost/json/parser.hpp>
#include <cstring>
#include <stdexcept>
#include <utility>

BOOST_JSON_NS_BEGIN

bool
parser::
handler::
on_document_begin(
    error_code&)
{
    return true;
}

bool
parser::
handler::
on_document_end(
    error_code&)
{
    return true;
}

bool
parser::
handler::
on_object_begin(
    error_code&)
{
    return true;
}

bool
parser::
handler::
on_object_end(
    std::size_t n,
    error_code&)
{
    st.push_object(n);
    return true;
}

bool
parser::
handler::
on_array_begin(
    error_code&)
{
    return true;
}

bool
parser::
handler::
on_array_end(
    std::size_t n,
    error_code&)
{
    st.push_array(n);
    return true;
}

bool
parser::
handler::
on_key_part(
    string_view s,
    error_code&)
{
    st.push_chars(s);
    return true;
}
        
bool
parser::
handler::
on_key(
    string_view s,
    error_code&)
{
    st.push_key(s);
    return true;
}
        
bool
parser::
handler::
on_string_part(
    string_view s,
    error_code&)
{
    st.push_chars(s);
    return true;
}

bool
parser::
handler::
on_string(
    string_view s,
    error_code&)
{
    st.push_string(s);
    return true;
}

bool
parser::
handler::
on_number_part(
    string_view,
    error_code&)
{
    return true;
}

bool
parser::
handler::
on_int64(
    std::int64_t i,
    string_view,
    error_code&)
{
    st.push_int64(i);
    return true;
}
        
bool
parser::
handler::
on_uint64(
    std::uint64_t u,
    string_view,
    error_code&)
{
    st.push_uint64(u);
    return true;
}

bool
parser::
handler::
on_double(
    double d,
    string_view,
    error_code&)
{
    st.push_double(d);
    return true;
}
        
bool
parser::
handler::
on_bool(
    bool b,
    error_code&)
{
    st.push_bool(b);
    return true;
}

bool
parser::
handler::
on_null(error_code&)
{
    st.push_null();
    return true;
}

bool
parser::
handler::
on_comment_part(
    string_view, error_code&)
{
    return true;
}
        
bool
parser::
handler::
on_comment(
    string_view, error_code&)
{
    return true;
}

//----------------------------------------------------------

parser::
parser(
    storage_ptr sp,
    parse_options const& opt,
    void* temp_buffer,
    std::size_t temp_size) noexcept
    : p_(
        opt,
        std::move(sp),
        temp_buffer,
        temp_size)
{
}

void
parser::
reset(storage_ptr sp) noexcept
{
    p_.reset();
    p_.handler().st.reset(sp);
}

std::size_t
parser::
write(
    char const* data,
    std::size_t size,
    error_code& ec)
{
    auto const n = p_.write(
        true, data, size, ec);
    if(! ec && n < size)
        ec = error::extra_data;
    return n;
}

void
parser::
finish(error_code& ec)
{
    p_.write(false, nullptr, 0, ec);
}

value
parser::
release(error_code& ec)
{
    if(p_.is_complete())
        return p_.handler().st.release();
    ec = error::incomplete;
    return nullptr;
}

BOOST_JSON_NS_END

#endif
