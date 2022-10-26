//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_SERIALIZE_HPP
#define BOOST_JSON_DETAIL_SERIALIZE_HPP

#include <boost/json/serializer.hpp>
#include <string>

namespace boost {
namespace json {
namespace detail {

BOOST_JSON_DECL
void
serialize_impl(
    std::string& s,
    serializer& sr);

} // detail
} // json
} // boost

#endif
