//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_SERIALIZE_HPP
#define BOOST_JSON_SERIALIZE_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/value.hpp>
#include <iosfwd>

BOOST_JSON_NS_BEGIN

/** Return a string representing a serialized element.

    This function serializes `t` as JSON and returns
    it as a @ref string using the specified memory
    resource.

    @par Complexity
    Constant or linear in the size of `t`.

    @par Exception Safety
    Strong guarantee.
    Calls to `memory_resource::allocate` may throw.

    @return The serialized string

    @param t The value to serialize

    @param sp The memory resource to use for the
    returned string. If omitted, the default resource
    is used.
*/
/** @{ */
BOOST_JSON_DECL
string
serialize(
    value const& t,
    storage_ptr sp = {});

BOOST_JSON_DECL
string
serialize(
    array const& t,
    storage_ptr sp = {});

BOOST_JSON_DECL
string
serialize(
    object const& t,
    storage_ptr sp = {});

BOOST_JSON_DECL
string
serialize(
    string const& t,
    storage_ptr sp = {});

BOOST_JSON_DECL
string
serialize(
    string_view t,
    storage_ptr sp = {});
/** @} */

/** Serialize an element to an output stream.

    This function serializes the specified element
    as JSON into the output stream.

    @return `os`.

    @par Complexity
    Constant or linear in the size of `t`.

    @par Exception Safety
    Strong guarantee.
    Calls to `memory_resource::allocate` may throw.

    @param os The output stream to serialize to.

    @param t The value to serialize
*/
/** @{ */
BOOST_JSON_DECL
std::ostream&
operator<<(
    std::ostream& os,
    value const& t);

BOOST_JSON_DECL
std::ostream&
operator<<(
    std::ostream& os,
    array const& t);

BOOST_JSON_DECL
std::ostream&
operator<<(
    std::ostream& os,
    object const& t);

BOOST_JSON_DECL
std::ostream&
operator<<(
    std::ostream& os,
    string const& t);
/** @} */

BOOST_JSON_NS_END

#endif
