//
// Copyright (c) 2021 Peter Dimov
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_PARSE_INTO_HPP
#define BOOST_JSON_PARSE_INTO_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/basic_parser_impl.hpp>
#include <boost/json/error.hpp>
#include <boost/json/string_view.hpp>
#include <boost/json/detail/parse_into.hpp>

#ifndef BOOST_JSON_STANDALONE

BOOST_JSON_NS_BEGIN

/** Type alias for parsing into a given type

    This type alias declares the type of a parser
    necessary to parse JSON directly into the user
    defined type `V`.

    @tparam V the type to parse into. This type
    must be <em>DefaultConstructible</em>.
*/
#ifdef BOOST_JSON_DOCS
template< class V >
using parser_for = __see_below__
#else
template< class V >
using parser_for =
    basic_parser<detail::into_handler<V>>;
#endif

/** Parse a JSON text into a user-defined object.

    @param v The type to parse into.

    @param sv The JSON text to parse.

    @param ec Set to the error if any occurred
*/
template<class V>
void
parse_into(
    V& v,
    string_view sv,
    error_code& ec );

BOOST_JSON_NS_END

#include <boost/json/impl/parse_into.hpp>

#endif

#endif
