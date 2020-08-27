//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_TO_STRING_HPP
#define BOOST_JSON_TO_STRING_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/value.hpp>
#include <iosfwd>

namespace boost {
namespace json {

/** Return a string representing a serialized @ref value.

    This function serializes the specified value
    and returns it as a @ref string using the
    default memory resource.

    @par Exception Safety

    Strong guarantee.
    Calls to `memory_resource::allocate` may throw.

    @return The serialized string

    @param jv The value to serialize
*/
BOOST_JSON_DECL
string
to_string(
    value const& jv);

/** Serialize a @ref value to an output stream.

    This function serializes the specified value
    into the output stream.

    @par Exception Safety

    Strong guarantee.
    Calls to `memory_resource::allocate` may throw.

    @return `os`.

    @param os The output stream to serialize to.

    @param jv The value to serialize.
*/
BOOST_JSON_DECL
std::ostream&
operator<<(
    std::ostream& os,
    value const& jv);

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/to_string.ipp>
#endif

#endif
