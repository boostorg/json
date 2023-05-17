//
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_PARSE_OPTIONS_HPP
#define BOOST_JSON_PARSE_OPTIONS_HPP

#include <boost/json/detail/config.hpp>

namespace boost {
namespace json {

/** Parser options

    This structure is used for specifying
    maximum parsing depth, and whether
    to allow various non-standard extensions.
    Default-constructed options set maximum
    parsing depth to 32 and specify that only
    standard JSON is allowed,

    @see
        @ref basic_parser,
        @ref parser.
*/
struct parse_options
{
    /** Maximum nesting level of arrays and objects.

        This specifies the maximum number of nested
        structures allowed while parsing a JSON text. If
        this limit is exceeded during a parse, an
        error is returned.

        @see
            @ref basic_parser,
            @ref stream_parser.
    */
    std::size_t max_depth = 32;

    /** Number prasing mode

        This selects enables precise parsing at the cost of performance.

        @see
            @ref basic_parser,
            @ref stream_parser.
    */
    bool precise_parsing = false;

    /** Non-standard extension option

        Allow C and C++ style comments to appear
        anywhere that whitespace is permissible.

        @see
            @ref basic_parser,
            @ref stream_parser.
    */
    bool allow_comments = false;

    /** Non-standard extension option

        Allow a trailing comma to appear after
        the last element of any array or object.

        @see
            @ref basic_parser,
            @ref stream_parser.
    */
    bool allow_trailing_commas = false;

    /** Non-standard extension option

        Allow invalid UTF-8 sequnces to appear
        in keys and strings.

        @note This increases parsing performance.

        @see
            @ref basic_parser,
            @ref stream_parser.
    */
    bool allow_invalid_utf8 = false;
};

} // namespace json
} // namespace boost

#endif
