//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IEEE_DECIMAL_HPP
#define BOOST_JSON_IEEE_DECIMAL_HPP

#include <boost/json/detail/config.hpp>

namespace boost {
namespace json {

struct ieee_decimal
{
    unsigned long long mantissa;
    short exponent;
    bool sign;
};

BOOST_JSON_DECL
double
to_double(ieee_decimal const& dec) noexcept;

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/ieee_decimal.ipp>
#endif

#endif
