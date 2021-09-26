//
// Copyright (c) 2021 Dmitry Arkhipov (grisumbras@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IOSTREAMS_HPP
#define BOOST_JSON_IOSTREAMS_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/error.hpp>
#include <boost/json/parse_options.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/json/stream_parser.hpp>
#include <boost/json/value.hpp>

#include <istream>

BOOST_JSON_NS_BEGIN

BOOST_JSON_DECL
value
parse(
    std::istream& is,
    error_code& ec,
    storage_ptr sp = {},
    parse_options const& opt = {});

BOOST_JSON_DECL
value
parse(
    std::streambuf* sb,
    error_code& ec,
    storage_ptr sp = {},
    parse_options const& opt = {});

BOOST_JSON_DECL
value
parse(
    std::istream& is,
    storage_ptr sp = {},
    parse_options const& opt = {});

class streambuf : public std::streambuf
{
    stream_parser p_;
    error_code ec_;
    memory_resource* mem_res_;

public:
    BOOST_JSON_DECL
    explicit
    streambuf(
        parse_options const& opt = {},
        storage_ptr sp = {},
        char* buffer = nullptr,
        std::size_t buffer_size = 0);

    BOOST_JSON_DECL
    ~streambuf() noexcept override;

    BOOST_JSON_DECL
    void
    reset_parser(storage_ptr sp = {}) noexcept;

    BOOST_JSON_DECL
    json::value
    value(error_code& ec);

    BOOST_JSON_DECL
    json::value
    value();

protected:
    BOOST_JSON_DECL
    int_type
    overflow(int_type ch = traits_type::eof()) override;

    BOOST_JSON_DECL
    std::streamsize
    xsputn(
        char const* s,
        std::streamsize count) override;
};

BOOST_JSON_NS_END

#endif // BOOST_JSON_IOSTREAMS_HPP
