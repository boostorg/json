//
// Copyright (c) 2021 Klemens D. Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_SERIALIZE_OPTIONS_HPP
#define BOOST_JSON_SERIALIZE_OPTIONS_HPP

#include <boost/json/detail/config.hpp>

BOOST_JSON_NS_BEGIN

/// Options for serialization of josn.
struct serialize_options
{
  /// The value to use for a positive `inf` double.
  /// The value will not be quoted in json.
  string_view pinf = "null";
  /// The value to use for a negative `inf` double.
  /// The value will not be quoted in json.
  string_view ninf = "null";
  /// The value to use for a NaN double.
  /// The value will not be quoted in json.
  string_view nan  = "null";
};

BOOST_JSON_NS_END

#endif //BOOST_JSON_SERIALIZE_OPTIONS_HPP
