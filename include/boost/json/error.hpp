//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_ERROR_HPP
#define BOOST_JSON_ERROR_HPP

#include <boost/json/detail/config.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace boost {
namespace json {

/// The type of error code used by the library
using error_code = boost::system::error_code;

/// The type of system error thrown by the library
using system_error = boost::system::system_error;

/// The type of error category used by the library
using error_category = boost::system::error_category;

/// The type of error condition used by the library
using error_condition = boost::system::error_condition;

/// Error codes returned by JSON operations
enum class error
{
    syntax = 1,

    /// Unexpected extra data encountered while parsing
    extra_data,

    /// A mantissa overflowed while parsing
    mantissa_overflow,

    /// The parser encountered an exponent that overflowed
    exponent_overflow,

    /// The parser's maximum depth limit was reached
    too_deep,

    /// Expected a value of kind object
    expected_object,

    /// Expected a value of kind array
    expected_array,

    /// Expected a value of kind string
    expected_string,

    /// Expect a value of kind number
    expected_number,

    /// Expected a value of kind boolean
    expected_bool,

    /// Expected a value of kind boolean
    expected_null,

    /// An integer assignment would overflow
    integer_overflow,

    /// The key was not found in the object
    key_not_found
};

/// Error conditions corresponding to JSON errors
enum class condition
{
    /// A parser-related error
    parse_error = 1,

    /// An error on assignment to or from a JSON value
    assign_error
};

} // json
} // boost

#include <boost/json/impl/error.hpp>
#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/error.ipp>
#endif

#endif
