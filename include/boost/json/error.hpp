//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_ERROR_HPP
#define BOOST_JSON_ERROR_HPP

#include <boost/json/detail/config.hpp>
#ifndef BOOST_JSON_STANDALONE
# include <boost/system/error_code.hpp>
# include <boost/system/system_error.hpp>
#else
# include <system_error>
#endif

namespace boost {
namespace json {

#ifndef BOOST_JSON_STANDALONE

/// The type of error code used by the library.
using error_code = boost::system::error_code;

/// The type of error category used by the library.
using error_category = boost::system::error_category;

/// The type of error condition used by the library.
using error_condition = boost::system::error_condition;

/// The type of system error thrown by the library.
using system_error = boost::system::system_error;

#ifdef BOOST_JSON_DOCS
/// Returns the generic error category used by the library.
error_category const&
generic_category();
#else
using boost::system::generic_category;
#endif

#else

using error_code = std::error_code;
using error_category = std::error_category;
using error_condition = std::error_condition;
using system_error = std::system_error;
using std::generic_category;

#endif

/** Error codes returned by JSON operations

*/
enum class error
{
    //----------------------------------

    /// syntax error
    syntax = 1,

    /// extra data
    extra_data,

    /// incomplete JSON
    incomplete,

    /// mantissa overflow
    mantissa_overflow,

    /// exponent too large
    exponent_overflow,

    /// too deep
    too_deep,

    /// illegal character for value
    illegal_char,

    /// illegal control character
    illegal_control_char,
    
    /// illegal character in escape sequence
    illegal_escape_char,

    /// illegal extra digits in number
    illegal_extra_digits,

    /// illegal leading surrogate
    illegal_leading_surrogate,

    /// illegal trailing surrogate
    illegal_trailing_surrogate,

    /** The parser needs to be started.
    */
    need_start,

    /// expected comma
    expected_comma,

    /// expected colon
    expected_colon,

    /// expected quotes
    expected_quotes,

    /// expected hex digit
    expected_hex_digit,

    /// expected utf16 escape
    expected_utf16_escape,

    /// expected mantissa
    expected_mantissa,

    /// expected fractional part of mantissa
    expected_fraction,

    /// expected exponent here
    expected_exponent,

    /// expected 'true'
    expected_true,

    /// expected 'false'
    expected_false,

    /// expected 'null'
    expected_null,

    /// not an object
    not_object,

    /// not an array
    not_array,

    /// not a string
    not_string,

    /// not a number
    not_number,

    /// not a boolean
    not_bool,

    /// not a null
    not_null,

    /// integer overflow
    integer_overflow,

    /// number cast is not exact
    not_exact,

    /// test failure
    test_failure,
};

/** Error conditions corresponding to JSON errors
*/
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
