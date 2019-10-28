//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_KIND_HPP
#define BOOST_JSON_KIND_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/detail/inline_variable.hpp>

namespace boost {
namespace json {

/// Constants for identifying the type of a @ref value
enum class kind
{
    /// An @ref object.
    object,

    /// An @ref array.
    array,

    /// A @ref string.
    string,

    /// A @ref number.
    number,

    /// A `bool`.
    boolean,

    /// A null.
    null
};

template<kind K>
struct kind_type
{
};

using kind_object_t = kind_type<kind::object>;
using kind_array_t = kind_type<kind::array>;
using kind_string_t = kind_type<kind::string>;
using kind_number_t = kind_type<kind::number>;
using kind_bool_t = kind_type<kind::boolean>;
using kind_null_t = kind_type<kind::null>;

BOOST_JSON_INLINE_VARIABLE(kind_object, kind_object_t);
BOOST_JSON_INLINE_VARIABLE(kind_array, kind_array_t);
BOOST_JSON_INLINE_VARIABLE(kind_string, kind_string_t);
BOOST_JSON_INLINE_VARIABLE(kind_number, kind_number_t);
BOOST_JSON_INLINE_VARIABLE(kind_bool, kind_bool_t);
BOOST_JSON_INLINE_VARIABLE(kind_null, kind_null_t);

} // json
} // boost

#endif
