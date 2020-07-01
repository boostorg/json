//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_DETAIL_FORMAT_HPP
#define BOOST_JSON_DETAIL_FORMAT_HPP

namespace boost {
namespace json {
namespace detail {

int constexpr max_number_chars =
     1 +    // '-'
    19 +    // unsigned 64-bit mantissa
     1 +    // 'e'
     1 +    // '-'
     5;     // unsigned 16-bit exponent

BOOST_JSON_DECL
unsigned
format_uint64(
    char* dest,
    std::uint64_t value) noexcept;

BOOST_JSON_DECL
unsigned
format_int64(
    char* dest, int64_t i) noexcept;

BOOST_JSON_DECL
unsigned
format_double(
    char* dest, double d) noexcept;

} // detail
} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/detail/impl/format.ipp>
#endif

#endif
