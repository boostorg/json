//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_PARSE_FILE_HPP
#define BOOST_JSON_PARSE_FILE_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/basic_parser.hpp>
#include <boost/beast/core/error.hpp>

namespace boost {
namespace beast {
namespace json {

/** Parse the contents of a file as JSON.
*/
BOOST_JSON_DECL
void
parse_file(
    char const* path,
    basic_parser& parser,
    error_code& ec);

} // json
} // beast
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/parse_file.ipp>
#endif

#endif
