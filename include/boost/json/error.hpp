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

#include <boost/json/config.hpp>
#include <stdexcept>

namespace boost {
namespace json {

/** Exception thrown when a value's kind is mismatched.
*/
struct BOOST_SYMBOL_VISIBLE
    type_error : std::invalid_argument
{
    /// Default constructor
    type_error()
        : std::invalid_argument(
            "type error") {}

    /** Constructor.

        Construct the exception using
        the specified message.

        @param what The exception message text.
    */
    explicit type_error(char const* what)
        : std::invalid_argument(what) {}
};

/** Exception thrown when a number is required.

    @see
        @ref value::as_int64,
        @ref value::as_uint64,
        @ref value::as_double
*/
struct BOOST_SYMBOL_VISIBLE
    number_required_error : type_error
{
    /// Default constructor.
    number_required_error()
        : type_error("number required") {}

    /** Constructor.

        Construct the exception using
        the specified message.

        @param what The exception message text.
    */
    explicit number_required_error(char const* what)
        : type_error(what) {}
};

//----------------------------------------------------------

/** Exception thrown when an object is required.

    @see @ref value::as_object
*/
struct BOOST_SYMBOL_VISIBLE
    object_required_error : type_error {
    /// Default constructor.
    object_required_error() : type_error(
        "object required") { } };

/** Exception thrown when an array is required.

    @see @ref value::as_array
*/
struct BOOST_SYMBOL_VISIBLE
    array_required_error : type_error {
    /// Default constructor.
    array_required_error() : type_error(
        "array required") { } };

/** Exception thrown when a string is required.

    @see @ref value::as_string
*/
struct string_required_error : type_error {
    string_required_error() : type_error(
        "string required") { } };

/** Exception thrown when a signed 64-bit integer is required.

    @see @ref value::as_int64
*/
struct BOOST_SYMBOL_VISIBLE
    int64_required_error : number_required_error {
    /// Default constructor.
    int64_required_error() : number_required_error(
        "int64 required") { } };

/** Exception thrown when an unsigned 64-bit integer is required.

    @see @ref value::as_uint64
*/
struct BOOST_SYMBOL_VISIBLE
    uint64_required_error : number_required_error {
    /// Default constructor.
    uint64_required_error() : number_required_error(
        "uint64 required") { } };

/** Exception thrown when a double is required.

    @see @ref value::as_double
*/
struct BOOST_SYMBOL_VISIBLE
    double_required_error : number_required_error {
    /// Default constructor.
    double_required_error() : number_required_error(
        "double required") { } };

/** Exception thrown when a bool is required.

    @see @ref value::as_bool
*/
struct BOOST_SYMBOL_VISIBLE
    bool_required_error : type_error {
    /// Default constructor.
    bool_required_error() : type_error(
        "bool required") { } };

/** Exception thrown when a key is too large.
*/
struct BOOST_SYMBOL_VISIBLE
    key_too_large : std::length_error {
    /// Default constructor
    key_too_large() : std::length_error(
        "key too large") { } };

/** Exception thrown when an array index is out of range.
*/
struct BOOST_SYMBOL_VISIBLE
    array_index_error : std::out_of_range {
    /// Default constructor
    array_index_error() : std::out_of_range(
        "array index error") { } };

/** Exception thrown when an object's maximum size would be exceeded.
*/
struct BOOST_SYMBOL_VISIBLE
    object_too_large : std::length_error {
    /// Default constructor
    object_too_large() : std::length_error(
        "object too large") { } };

/** Exception thrown when an array's maximum size would be exceeded.
*/
struct BOOST_SYMBOL_VISIBLE
    array_too_large : std::length_error {
    /// Default constructor
    array_too_large() : std::length_error(
        "array too large") { } };

/** Exception thrown when a string's maximum size would be exceeded.
*/
struct BOOST_SYMBOL_VISIBLE
    string_too_large : std::length_error {
    /// Default constructor
    string_too_large() : std::length_error(
        "string too large") { } };

/** Exception thrown when a character offset is out of range.
*/
struct BOOST_SYMBOL_VISIBLE
    char_pos_error : std::out_of_range {
    /// Default constructor
    char_pos_error() : std::out_of_range(
        "char index error") { } };

/** Exception thrown when a key is not find in an object.
*/
struct BOOST_SYMBOL_VISIBLE
    key_not_found : std::invalid_argument {
    /// Default constructor
    key_not_found() : std::invalid_argument(
        "key not found") { } };

/** Exception thrown when a stack limit is exceeded.
*/
struct BOOST_SYMBOL_VISIBLE
    stack_overflow : std::runtime_error {
    /// Default constructor
    stack_overflow() : std::runtime_error(
        "stack overflow") { } };

//----------------------------------------------------------

/** Error codes returned by JSON operations

*/
enum class error
{
    /// syntax error
    syntax = 1,

    /// extra data
    extra_data,

    /// incomplete data
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
    test_failure
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
