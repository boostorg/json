//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_KIND_HPP
#define BOOST_JSON_KIND_HPP

#include <boost/json/config.hpp>

namespace boost {
namespace json {

/// Constants for identifying the type of a value
enum class kind : unsigned char
{
    /// An @ref object.
    object,

    /// An @ref array.
    array,

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
