//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_KIND_HPP
#define BOOST_JSON_KIND_HPP

#include <boost/json/detail/config.hpp>

BOOST_JSON_NS_BEGIN

/** Constants for identifying the underlying representation of a value

    These values are returned from @ref value::kind
*/
enum class kind : unsigned char
{
    /// An @ref array.
    array,

    /// An @ref object.
    object,

    /// A @ref string.
    string,

    /// A `std::int64_t`
    int64,

    /// A `std::uint64_t`
    uint64,

    /// A `double`.
    double_,

    /// A `bool`.
    bool_,

    /// The null value.
    null
};

/** A tag type used to select a @ref value constructor overload.

    The library provides the constant @ref array_kind
    which may be used to select the @ref value constructor
    that creates an empty @ref array.

    @see @ref array_kind
*/
struct array_kind_t
{
};

/** A tag type used to select a @ref value constructor overload.

    The library provides the constant @ref object_kind
    which may be used to select the @ref value constructor
    that creates an empty @ref object.

    @see @ref object_kind
*/
struct object_kind_t
{
};

/** A tag type used to select a @ref value constructor overload.

    The library provides the constant @ref string_kind
    which may be used to select the @ref value constructor
    that creates an empty @ref string.

    @see @ref string_kind
*/
struct string_kind_t
{
};

/** A constant used to select a @ref value constructor overload.

    The library provides this constant to allow efficient
    construction of a @ref value containing an empty @ref array.

    @par Example
    @code
    storage_ptr sp;
    value jv( array_kind, sp ); // sp is an optional parameter
    @endcode

    @see @ref array_kind_t
*/
BOOST_JSON_INLINE_VARIABLE(array_kind, array_kind_t);

/** A constant used to select a @ref value constructor overload.

    The library provides this constant to allow efficient
    construction of a @ref value containing an empty @ref object.

    @par Example
    @code
    storage_ptr sp;
    value jv( object_kind, sp ); // sp is an optional parameter
    @endcode

    @see @ref object_kind_t
*/
BOOST_JSON_INLINE_VARIABLE(object_kind, object_kind_t);

/** A constant used to select a @ref value constructor overload.

    The library provides this constant to allow efficient
    construction of a @ref value containing an empty @ref string.

    @par Example
    @code
    storage_ptr sp;
    value jv( string_kind, sp ); // sp is an optional parameter
    @endcode

    @see @ref string_kind_t
*/
BOOST_JSON_INLINE_VARIABLE(string_kind, string_kind_t);

BOOST_JSON_NS_END

#endif
