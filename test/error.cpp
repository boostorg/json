//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json/error.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <memory>

namespace boost {
namespace json {

class error_test : public beast::unit_test::suite
{
public:
    void check(error e)
    {
        auto const ec = make_error_code(e);
        ec.category().name();
        BEAST_EXPECT(! ec.message().empty());
        BEAST_EXPECT(ec.category().default_error_condition(
            static_cast<int>(e)).category() == ec.category());
    }

    void check(condition c, error e)
    {
        {
            auto const ec = make_error_code(e);
            BEAST_EXPECT(ec.category().name() != nullptr);
            BEAST_EXPECT(! ec.message().empty());
            BEAST_EXPECT(ec == c);
        }
        {
            auto ec = make_error_condition(c);
            BEAST_EXPECT(ec.category().name() != nullptr);
            BEAST_EXPECT(! ec.message().empty());
            BEAST_EXPECT(ec == c);
        }
    }

    void run() override
    {
        check(condition::parse_error, error::syntax);
        check(condition::parse_error, error::extra_data);
        check(condition::parse_error, error::mantissa_overflow);
        check(condition::parse_error, error::exponent_overflow);
        check(condition::parse_error, error::too_deep);
        
        check(condition::parse_error, error::illegal_char);
        check(condition::parse_error, error::illegal_control_char);
        check(condition::parse_error, error::illegal_escape_char);
        check(condition::parse_error, error::illegal_extra_digits);
        check(condition::parse_error, error::illegal_extra_chars);
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

        check(condition::assign_error, error::not_object);
        check(condition::assign_error, error::not_array);
        check(condition::assign_error, error::not_string);
        check(condition::assign_error, error::not_number);
        check(condition::assign_error, error::not_bool);
        check(condition::assign_error, error::not_null);
        check(condition::assign_error, error::integer_overflow);
    
        check(error::need_start);
        check(error::key_not_found);
        check(error::test_failure);
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,error);

} // json
} // boost
