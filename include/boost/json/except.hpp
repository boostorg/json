//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_EXCEPT_HPP
#define BOOST_JSON_EXCEPT_HPP

#include <boost/json/detail/config.hpp>
#include <stdexcept>

namespace boost {
namespace json {

/** Exception thrown when a value's kind is mismatched.
*/
struct BOOST_SYMBOL_VISIBLE
    type_error : std::invalid_argument
{
    /** Constructor.

        Construct the exception using
        the specified message.

        @param what The exception message text.
    */
    BOOST_JSON_DECL
    explicit
    type_error(char const* what);
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
    /** Constructor.

        Construct the exception using
        the specified message.

        @param what The exception message text.
    */
    BOOST_JSON_DECL
    explicit
    number_required_error(char const* what);
};

//----------------------------------------------------------

/** Exception thrown when an array index is out of range.
*/
struct BOOST_SYMBOL_VISIBLE
    array_index_error : std::out_of_range
{
    /// Default constructor
    BOOST_JSON_DECL
    array_index_error();

#ifndef BOOST_JSON_DOCS
    BOOST_JSON_DECL
    static BOOST_NORETURN void raise();
#endif
};

/** Exception thrown when an array is required.

    @see @ref value::as_array
*/
struct BOOST_SYMBOL_VISIBLE
    array_required_error : type_error
{
    /// Default constructor.
    BOOST_JSON_DECL
    array_required_error();

#ifndef BOOST_JSON_DOCS
    BOOST_JSON_DECL
    static BOOST_NORETURN void raise();
#endif
};

/** Exception thrown when an array's maximum size would be exceeded.
*/
struct BOOST_SYMBOL_VISIBLE
    array_too_large : std::length_error
{
    /// Default constructor
    BOOST_JSON_DECL
    array_too_large();

#ifndef BOOST_JSON_DOCS
    BOOST_JSON_DECL
    static BOOST_NORETURN void raise();
#endif
};

/** Exception thrown when a bool is required.

    @see @ref value::as_bool
*/
struct BOOST_SYMBOL_VISIBLE
    bool_required_error : type_error
{
    /// Default constructor.
    BOOST_JSON_DECL
    bool_required_error();

#ifndef BOOST_JSON_DOCS
    BOOST_JSON_DECL
    static BOOST_NORETURN void raise();
#endif
};

/** Exception thrown when a character offset is out of range.
*/
struct BOOST_SYMBOL_VISIBLE
    char_pos_error : std::out_of_range
{
    /// Default constructor
    BOOST_JSON_DECL
    char_pos_error();

#ifndef BOOST_JSON_DOCS
    BOOST_JSON_DECL
    static BOOST_NORETURN void raise();
#endif
};

/** Exception thrown when a double is required.

    @see @ref value::as_double
*/
struct BOOST_SYMBOL_VISIBLE
    double_required_error : number_required_error
{
    /// Default constructor.
    BOOST_JSON_DECL
    double_required_error();

#ifndef BOOST_JSON_DOCS
    BOOST_JSON_DECL
    static BOOST_NORETURN void raise();
#endif
};

/** Exception thrown when a signed 64-bit integer is required.

    @see @ref value::as_int64
*/
struct BOOST_SYMBOL_VISIBLE
    int64_required_error : number_required_error
{
    /// Default constructor.
    BOOST_JSON_DECL
    int64_required_error();

#ifndef BOOST_JSON_DOCS
    BOOST_JSON_DECL
    static BOOST_NORETURN void raise();
#endif
};

/** Exception thrown when a key is not find in an object.
*/
struct BOOST_SYMBOL_VISIBLE
    key_not_found : std::invalid_argument
{
    /// Default constructor
    BOOST_JSON_DECL
    key_not_found();

#ifndef BOOST_JSON_DOCS
    BOOST_JSON_DECL
    static BOOST_NORETURN void raise();
#endif
};

/** Exception thrown when a key is too large.
*/
struct BOOST_SYMBOL_VISIBLE
    key_too_large : std::length_error
{
    /// Default constructor
    BOOST_JSON_DECL
    key_too_large();

#ifndef BOOST_JSON_DOCS
    BOOST_JSON_DECL
    static BOOST_NORETURN void raise();
#endif
};

/** Exception thrown when an object is required.

    @see @ref value::as_object
*/
struct BOOST_SYMBOL_VISIBLE
    object_required_error : type_error
{
    /// Default constructor.
    BOOST_JSON_DECL
    object_required_error();

#ifndef BOOST_JSON_DOCS
    BOOST_JSON_DECL
    static BOOST_NORETURN void raise();
#endif
};

/** Exception thrown when an object's maximum size would be exceeded.
*/
struct BOOST_SYMBOL_VISIBLE
    object_too_large : std::length_error
{
    /// Default constructor
    BOOST_JSON_DECL
    object_too_large();

#ifndef BOOST_JSON_DOCS
    BOOST_JSON_DECL
    static BOOST_NORETURN void raise();
#endif
};

/** Exception thrown when a stack limit is exceeded.
*/
struct BOOST_SYMBOL_VISIBLE
    stack_overflow : std::runtime_error
{
    /// Default constructor
    BOOST_JSON_DECL
    stack_overflow();

#ifndef BOOST_JSON_DOCS
    BOOST_JSON_DECL
    static BOOST_NORETURN void raise();
#endif
};

/** Exception thrown when a string is required.

    @see @ref value::as_string
*/
struct BOOST_SYMBOL_VISIBLE
    string_required_error : type_error
{
    /// Default constructor.
    BOOST_JSON_DECL
    string_required_error();

#ifndef BOOST_JSON_DOCS
    BOOST_JSON_DECL
    static BOOST_NORETURN void raise();
#endif
};

/** Exception thrown when a string's maximum size would be exceeded.
*/
struct BOOST_SYMBOL_VISIBLE
    string_too_large : std::length_error
{
    /// Default constructor
    BOOST_JSON_DECL
    string_too_large();

#ifndef BOOST_JSON_DOCS
    BOOST_JSON_DECL
    static BOOST_NORETURN void raise();
#endif
};

/** Exception thrown when an unsigned 64-bit integer is required.

    @see @ref value::as_uint64
*/
struct BOOST_SYMBOL_VISIBLE
    uint64_required_error : number_required_error
{
    /// Default constructor.
    BOOST_JSON_DECL
    uint64_required_error();

#ifndef BOOST_JSON_DOCS
    BOOST_JSON_DECL
    static BOOST_NORETURN void raise();
#endif
};

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/except.ipp>
#endif

#endif
