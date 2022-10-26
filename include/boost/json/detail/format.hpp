//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_FORMAT_HPP
#define BOOST_JSON_DETAIL_FORMAT_HPP

#include <boost/json/string_view.hpp>

namespace boost {
namespace json {
namespace detail {

int constexpr max_number_chars =
     1 +    // '-'
    20 +    // unsigned 64-bit mantissa
     1 +    // 'e'
     1 +    // '-'
     5;     // unsigned 16-bit exponent

string_view
write_int64(
    char* temp,
    std::size_t size,
    std::int64_t v) noexcept;

string_view
write_uint64(
    char* temp,
    std::size_t size,
    std::uint64_t v) noexcept;

string_view
write_double(
    char* temp,
    std::size_t size,
    double v) noexcept;

} // detail
} // namespace json
} // namespace boost

#endif
