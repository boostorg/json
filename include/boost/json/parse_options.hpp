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

/** Parser Extensions.
    
    The parser can be configured to enable
    certain extensions. By default, only conformant
    JSON is accepted.
    
    The currently supported extensions are:
    
    <li> Comments, which permit comments
    to appear within whitespace, and

    <li> Trailing commas, which allow a comma
    to appear after the last element of an object or array.
*/
struct parse_options
{
#if __cplusplus < 201402L
    parse_options(
        bool comments = false, 
        bool commas = false) noexcept
        : allow_comments(comments)
        , allow_trailing_commas(commas)
    {
    }

    bool allow_comments;
    bool allow_trailing_commas;
#else
    bool allow_comments = false;
    bool allow_trailing_commas = false;
#endif
};

#endif
