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

BOOST_JSON_NS_BEGIN
namespace detail {

std::size_t constexpr max_number_chars =
     1 +    // '-'
    19 +    // unsigned 64-bit mantissa
     1 +    // 'e'
     1 +    // '-'
     5;     // unsigned 16-bit exponent

std::size_t constexpr max_number_chars_for(const error_code & ) { return max_number_chars;}
std::size_t constexpr max_number_chars_for(const serialize_options & opt )
{
    return (std::max)({max_number_chars, opt.nan.size(), opt.ninf.size(), opt.pinf.size() });
}

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
    char* dest, double d, const serialize_options &) noexcept;

BOOST_JSON_DECL
unsigned
    format_double(
    char* dest, double d, error_code &) noexcept;

} // detail
BOOST_JSON_NS_END

#endif
