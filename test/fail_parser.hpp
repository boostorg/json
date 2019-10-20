//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_TEST_FAIL_PARSER_HPP
#define BOOST_JSON_TEST_FAIL_PARSER_HPP

#include <boost/json/basic_parser.hpp>

namespace boost {
namespace json {

class fail_parser
    : public basic_parser
{
    std::size_t n_ = std::size_t(-1);
    char buf[256];

    void
    maybe_fail(error_code& ec)
    {
        if(n_ && --n_ > 0)
            return;
        ec = error::test_failure;
    }

    void
    on_stack_info(
        stack& s) noexcept override
    {
        s.base = buf;
        s.capacity = sizeof(buf);
    }

    void
    on_stack_grow(
        stack&,
        unsigned,
        error_code& ec) override
    {
        ec = error::too_deep;
    }

    void
    on_document_begin(
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_object_begin(
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_object_end(
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_array_begin(
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_array_end(
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_key_data(
        string_view,
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_key_end(
        string_view,
        error_code& ec) override
    {
        maybe_fail(ec);
    }
        
    void
    on_string_data(
        string_view,
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_string_end(
        string_view,
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_number(
        ieee_decimal,
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_bool(
        bool,
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_null(error_code& ec) override
    {
        maybe_fail(ec);
    }

public:
    fail_parser() = default;

    explicit
    fail_parser(
        std::size_t n)
        : n_(n)
    {
    }
};

} // json
} // boost

#endif
