//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

// Test that header file is self-contained.
#include <boost/json/error.hpp>

#include <memory>

#include "test_suite.hpp"

namespace boost {
namespace json {

class error_test
{
public:
    void check(error e)
    {
        auto const ec = make_error_code(e);
        BOOST_TEST(ec.category().name() != nullptr);
        BOOST_TEST(! ec.message().empty());
        BOOST_TEST(ec.category().default_error_condition(
            static_cast<int>(e)).category() == ec.category());
    }

    void check(condition c, error e)
    {
        {
            auto const ec = make_error_code(e);
            BOOST_TEST(ec.category().name() != nullptr);
            BOOST_TEST(! ec.message().empty());
            BOOST_TEST(ec == c);
        }
        {
            auto ec = make_error_condition(c);
            BOOST_TEST(ec.category().name() != nullptr);
            BOOST_TEST(! ec.message().empty());
            BOOST_TEST(ec == c);
        }
    }

    void
    run()
    {
        check(condition::parse_error, error::syntax);
        check(condition::parse_error, error::extra_data);
        check(condition::parse_error, error::incomplete);
        check(condition::parse_error, error::mantissa_overflow);
        check(condition::parse_error, error::exponent_overflow);
        check(condition::parse_error, error::too_deep);
        
        check(condition::parse_error, error::illegal_char);
        check(condition::parse_error, error::illegal_control_char);
        check(condition::parse_error, error::illegal_escape_char);
        check(condition::parse_error, error::illegal_extra_digits);
        check(condition::parse_error, error::illegal_leading_surrogate);
        check(condition::parse_error, error::illegal_trailing_surrogate);

        check(condition::parse_error, error::expected_comma);
        check(condition::parse_error, error::expected_colon);
        check(condition::parse_error, error::expected_quotes);
        check(condition::parse_error, error::expected_hex_digit);
        check(condition::parse_error, error::expected_utf16_escape);
        check(condition::parse_error, error::expected_mantissa);
        check(condition::parse_error, error::expected_fraction);
        check(condition::parse_error, error::expected_exponent);
        check(condition::parse_error, error::expected_true);
        check(condition::parse_error, error::expected_false);
        check(condition::parse_error, error::expected_null);
        check(condition::parse_error, error::object_too_large);
        check(condition::parse_error, error::array_too_large);
        check(condition::parse_error, error::key_too_large);
        check(condition::parse_error, error::string_too_large);

        check(condition::assign_error, error::not_object);
        check(condition::assign_error, error::not_array);
        check(condition::assign_error, error::not_string);
        check(condition::assign_error, error::not_number);
        check(condition::assign_error, error::not_bool);
        check(condition::assign_error, error::not_null);
        check(condition::assign_error, error::integer_overflow);
        check(condition::assign_error, error::not_exact);
    
        check(error::need_start);
        check(error::test_failure);
    }
};

TEST_SUITE(error_test, "boost.json.error");

} // json
} // boost
