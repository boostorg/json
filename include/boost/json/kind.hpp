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

namespace boost {
namespace json {

/// Constants for identifying the type of a value
enum class kind
{
    /// An @ref object.
    object,

    /// An @ref array.
    array,

    /// A @ref string.
    string,

    /// A signed 64-bit integer.
    int64,

    /// An unsigned 64-bit integer.
    uint64,

    /// A double precision floating point.
    double_,

    /// A `bool`.
    boolean,

    /// A null.
    null
};

struct object_kind_t
{
};

struct array_kind_t
{
};

struct string_kind_t
{
};

BOOST_JSON_INLINE_VARIABLE(object_kind, object_kind_t);
BOOST_JSON_INLINE_VARIABLE(array_kind, array_kind_t);
BOOST_JSON_INLINE_VARIABLE(string_kind, string_kind_t);

} // json
} // boost

#endif
