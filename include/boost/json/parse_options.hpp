//
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_PARSE_OPTIONS_HPP
#define BOOST_JSON_PARSE_OPTIONS_HPP

namespace boost {
namespace json {

/** Parser Extensions.
    
    The parser can be configured to enable
    certain extensions. By default, only conformant
    JSON is accepted.
    
    The currently supported extensions are:
    
    @li Comments, which permits comments
    to appear within whitespace, and

    @li Trailing commas, which allows a comma
    to appear after the last element
    of an object or array, and

    @li Invalid UTF-8, which disables the
    validation of UTF-8 byte sequences.
*/
struct parse_options
{
    /** Comments

        Permits C and C++ style comments
        to appear within whitespace.
    */
    bool allow_comments = false;
    
    /** Trailing Commas
        
        Permits a comma to appear after the
        last element of an array or object.
    */
    bool allow_trailing_commas = false;
    
    /** Invalid UTF-8

        UTF-8 byte sequences will not
        be checked for validity.
    */
    bool allow_invalid_utf8 = false;
};

} // json
} // boost

#endif
