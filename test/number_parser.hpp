//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_TEST_NUMBER_PARSER_HPP
#define BOOST_JSON_TEST_NUMBER_PARSER_HPP

#include <boost/json/basic_parser.hpp>
#include <stdexcept>

namespace boost {
namespace json {

inline
ieee_decimal
parse_ieee_decimal(string_view s)
{
    struct number_parser : basic_parser
    {
        std::size_t n_ = std::size_t(-1);
        char buf[256];

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
            error_code&) override
        {
        }

        void
        on_object_begin(
            error_code&) override
        {
        }

        void
        on_object_end(
            error_code&) override
        {
        }

        void
        on_array_begin(
            error_code&) override
        {
        }

        void
        on_array_end(
            error_code&) override
        {
        }

        void
        on_key_data(
            string_view,
            error_code&) override
        {
        }

        void
        on_key_end(
            string_view,
            error_code&) override
        {
        }
        
        void
        on_string_data(
            string_view,
            error_code&) override
        {
        }

        void
        on_string_end(
            string_view,
            error_code&) override
        {
        }

        void
        on_number(
            ieee_decimal dec_,
            error_code&) override
        {
            dec = dec_;
        }

        void
        on_bool(
            bool,
            error_code&) override
        {
        }

        void
        on_null(error_code&) override
        {
        }

        ieee_decimal dec;
        number_parser() = default;
    };
    error_code ec;
    number_parser p;
    p.write(s.data(), s.size(), ec);
    if(ec)
        throw system_error(ec);
    return p.dec;
}

} // json
} // boost

#endif
