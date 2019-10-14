//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
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
            case error::syntax: return
                "The serialized JSON object contains a syntax error";

            case error::extra_data: return
                "Unexpected extra data encountered while parsing";

            case error::mantissa_overflow: return
                "A mantissa overflowed while parsing";

            case error::exponent_overflow: return
                "An exponent overflowed while parsing";

            case error::too_deep: return
                "The parser reached the maximum allowed depth";

            //

            case error::integer_overflow: return
                "An integer assignment overflowed";

            case error::expected_object: return
                "Expected a value of kind object";

            case error::expected_array: return
                "Expected a value of kind array";

            case error::expected_string: return
                "Expected a value of kind string";

            case error::expected_number: return
                "Expected a value of kind number";

            case error::expected_bool: return
                "Expected a value of kind bool";

            case error::expected_null: return
                "Expected a value of kind null";

            //

            case error::key_not_found: return
                "The key was not found in the object";
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
            case error::mantissa_overflow:
            case error::exponent_overflow:
            case error::too_deep:
                return condition::parse_error;

            case error::integer_overflow:
            case error::expected_object:
            case error::expected_array:
            case error::expected_string:
            case error::expected_number:
            case error::expected_bool:
            case error::expected_null:
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
                return "A JSON parsing error occurred";
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
