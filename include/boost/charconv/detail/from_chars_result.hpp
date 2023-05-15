// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_FROM_CHARS_RESULT_HPP
#define BOOST_CHARCONV_DETAIL_FROM_CHARS_RESULT_HPP

namespace boost { namespace charconv {

// 22.13.3, Primitive numerical input conversion

struct from_chars_result
{
    const char* ptr;

    // Values:
    // 0 = no error
    // EINVAL = invalid_argument
    // ERANGE = result_out_of_range
    int ec;

    friend constexpr bool operator==(const from_chars_result& lhs, const from_chars_result& rhs) noexcept
    {
        return lhs.ptr == rhs.ptr && lhs.ec == rhs.ec;
    }

    friend constexpr bool operator!=(const from_chars_result& lhs, const from_chars_result& rhs) noexcept
    {
        return !(lhs == rhs);
    }
};

}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_FROM_CHARS_RESULT_HPP
