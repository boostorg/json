//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_IEEE_DECIMAL_IPP
#define BOOST_JSON_IMPL_IEEE_DECIMAL_IPP

#include <boost/json/detail/config.hpp>
#include <boost/json/detail/math.hpp>

namespace boost {
namespace json {

namespace detail {

} // detail

/*
struct ieee_decimal
{
    unsigned long long mantissa;
    short exponent;
    bool sign;
};
*/

double
to_double(
    ieee_decimal const& dec) noexcept
{
    auto const fp =
        [](double m, int e) -> double
    {
        if(e < -308)
            return 0.0;
        if(e >= 0)
            return m * detail::pow10(e);
        else
            return m / detail::pow10(-e);
    };

    double d = static_cast<
        double>(dec.mantissa);
    if(dec.exponent < -308)
    {
        d = fp(d, -308);
        d = fp(d, dec.exponent + 308);
    }
    else
    {
        d = fp(d, dec.exponent);
    }
    return d;
}

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/ieee_decimal.ipp>
#endif

#endif
