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

/** Parser options

    This structure is used for specifying whether
    to allow various non-standard extensions.
    Default-constructed options specify that only
    standard JSON is allowed.
*/
struct parse_options
{
    /** Non-standard extension option

        Allow C and C++ style comments to appear
        anywhere that whitespace is permissible.
    */
    bool allow_comments = false;

    /** Non-standard extension option

        Allow a trailing comma to appear after
        the last element of any array or object.
    */
    bool allow_trailing_commas = false;

    /** Non-standard extension option

        Allow invalid UTF-8 sequnces to appear
        in keys and strings.

        @note This increases parsing performance.
    */
    bool allow_invalid_utf8 = false;
};

} // json
} // boost

#endif
