// Copyright 2023 Matt Borland
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT80_HPP
#define BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT80_HPP

#include <boost/charconv/detail/config.hpp>
#include <limits>
#include <cstdint>
#include <cmath>

namespace boost { namespace charconv { namespace detail {

inline long double compute_float80(std::int64_t power, std::uint64_t i, bool negative, bool& success) noexcept
{
    long double return_val;

    // At the absolute minimum and maximum rounding errors of 1 ULP can cause overflow
    if (power == 4914 && i == UINT64_C(1189731495357231765))
    {
        return_val = std::numeric_limits<long double>::max();
    }
    else if (power == -4950 && i == UINT64_C(3362103143112093506))
    {
        return_val = std::numeric_limits<long double>::min();
    }
    else
    {
        return_val = i * std::pow(10.0L, static_cast<long double>(power));
        if (std::isinf(return_val))
        {
            success = false;
            return negative ? -0.0L : 0.0L;
        }
    }

    return_val = negative ? -return_val : return_val;

    success = true;
    return return_val;
}

}}} // Namespaces

#endif // BOOST_CHARCONV_DETAIL_COMPUTE_FLOAT80_HPP
