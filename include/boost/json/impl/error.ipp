//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_ERROR_IPP
#define BOOST_JSON_IMPL_ERROR_IPP

#include <boost/json/error.hpp>

namespace boost {
namespace json {

error_code
make_error_code(error e)
{
    struct codes : error_category
    {
        const char*
        name() const noexcept override
        {
            return "boost.json";
        }

        std::string
        message(int ev) const override
        {
            switch(static_cast<error>(ev))
            {
            default:
case error::syntax: return "syntax error";
case error::extra_data: return "extra data";
case error::incomplete: return "incomplete JSON";
case error::mantissa_overflow: return "mantissa overflow";
case error::exponent_overflow: return "exponent overflow";
case error::too_deep: return "too deep";
case error::illegal_char: return "illegal character for value";
case error::illegal_control_char: return "illegal control character";
case error::illegal_escape_char: return "illegal character in escape sequence";
case error::illegal_extra_digits: return "illegal extra digits in number";
case error::illegal_leading_surrogate: return "illegal leading surrogate";
case error::illegal_trailing_surrogate: return "illegal trailing surrogate";
case error::need_start: return "parser needs start";

case error::expected_comma: return "expected comma";
case error::expected_colon: return "expected colon";
case error::expected_quotes: return "expected quotes";
case error::expected_hex_digit: return "expected hex digit";
case error::expected_utf16_escape: return "expected utf16 escape";
case error::expected_mantissa: return "expected mantissa";
case error::expected_fraction: return "expected mantissa fraction";
case error::expected_exponent: return "expected exponent";
case error::expected_true: return "expected 'true'";
case error::expected_false: return "expected 'false'";
case error::expected_null: return "expected 'null'";

case error::not_object: return "not an object";
case error::not_array: return "not an array";
case error::not_string: return "not a string";
case error::not_number: return "not a number";
case error::not_bool: return "not a boolean";
case error::not_null: return "not a null";

case error::integer_overflow: return "integer overflowed";
case error::not_exact: return "not exact";

case error::object_too_large: return "object too large";
case error::array_too_large: return "array too large";
case error::key_too_large: return "key too large";
case error::string_too_large: return "string too large";

case error::test_failure: return "test failure";
            }
        }

        error_condition
        default_error_condition(
            int ev) const noexcept override
        {
            switch(static_cast<error>(ev))
            {
            default:
                return {ev, *this};

case error::syntax:
case error::extra_data:
case error::incomplete:
case error::mantissa_overflow:
case error::exponent_overflow:
case error::too_deep:
case error::illegal_char:
case error::illegal_control_char:
case error::illegal_escape_char:
case error::illegal_extra_digits:
case error::illegal_leading_surrogate:
case error::illegal_trailing_surrogate:
case error::expected_comma:
case error::expected_colon:
case error::expected_quotes:
case error::expected_hex_digit:
case error::expected_utf16_escape:
case error::expected_mantissa:
case error::expected_fraction:
case error::expected_exponent:
case error::expected_true:
case error::expected_false:
case error::expected_null:
case error::object_too_large:
case error::array_too_large:
case error::key_too_large:
case error::string_too_large:
    return condition::parse_error;

case error::not_object:
case error::not_array:
case error::not_string:
case error::not_number:
case error::not_bool:
case error::not_null:
case error::integer_overflow:
case error::not_exact:
    return condition::assign_error;
            }
        }
    };

    static codes const cat{};
    return error_code{static_cast<
        std::underlying_type<error>::type>(e), cat};
}

error_condition
make_error_condition(condition c)
{
    struct codes : error_category
    {
        const char*
        name() const noexcept override
        {
            return "boost.json";
        }

        std::string
        message(int cv) const override
        {
            switch(static_cast<condition>(cv))
            {
            default:
            case condition::parse_error:
                return "A JSON parse error occurred";
            case condition::assign_error:
                return "An error occurred during assignment";
            }
        }
    };
    static codes const cat{};
    return error_condition{static_cast<
        std::underlying_type<condition>::type>(c), cat};
}

} // json
} // boost

#endif
