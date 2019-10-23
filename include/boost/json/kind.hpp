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

//[usage_a_1

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

//]

} // json
} // boost

#endif
