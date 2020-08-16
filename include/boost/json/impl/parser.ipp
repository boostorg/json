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

namespace boost {
namespace json {

parser::
parser() noexcept
    : parser(
        storage_ptr(),
        parse_options())
{
}

parser::
parser(
    const parse_options& opt) noexcept
    : parser(storage_ptr(), opt)
{
}

parser::
parser(
    storage_ptr sp) noexcept
    : parser(
        std::move(sp),
        parse_options())
{
}

parser::
parser(
    storage_ptr sp,
    const parse_options& opt) noexcept
    : p_(opt, std::move(sp))
{
}

void
parser::
reserve(std::size_t n)
{
    p_.handler().vb.reserve(n);
}

void
parser::
reset(storage_ptr sp) noexcept
{
    p_.handler().vb.reset(sp);
}

void
parser::
clear() noexcept
{
    p_.reset();
    p_.handler().vb.clear();
}

std::size_t
parser::
write(
    char const* data,
    std::size_t size,
    error_code& ec)
{
    auto const n = p_.write_some(
        true, data, size, ec);
    if(! ec && n < size)
        ec = error::extra_data;
    return n;
}

std::size_t
parser::
write(
    char const* data,
    std::size_t size)
{
    error_code ec;
    auto const n =
        write(data, size, ec);
    if(ec)
        BOOST_THROW_EXCEPTION(
            system_error(ec));
    return n;
}

void
parser::
finish(error_code& ec)
{
    p_.write_some(
        false, nullptr, 0, ec);
}

void
parser::
finish()
{
    write(nullptr, 0);
}

value
parser::
release()
{
    /*
    if(! p_.is_complete())
        BOOST_THROW_EXCEPTION(
            std::logic_error(
                "no value"));
    */
    return p_.handler().vb.release();
}

//----------------------------------------------------------

value
parse(
    string_view s,
    error_code& ec,
    storage_ptr sp)
{
    parser p;
    p.reset(std::move(sp));
    p.write(
        s.data(),
        s.size(),
        ec);
    if(! ec)
        p.finish(ec);
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
