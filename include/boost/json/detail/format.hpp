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

#include <boost/charconv/limits.hpp>
#include <boost/json/detail/config.hpp>

namespace boost {
namespace json {
namespace detail {

constexpr std::size_t max_number_chars =
    boost::charconv::limits<double>::max_chars;

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
std::size_t
format_double(
    char* dest, double d, bool allow_infinity_and_nan = false) noexcept;

} // detail
} // namespace json
} // namespace boost

#endif
