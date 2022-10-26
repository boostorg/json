//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_SERIALIZE_HPP
#define BOOST_JSON_IMPL_SERIALIZE_HPP

#include <boost/json/serializer.hpp>
#include <boost/json/detail/serialize.hpp>

namespace boost {
namespace json {

template<class T>
std::string
serialize(T const& t)
{
    std::string s;
    serializer sr;
    sr.reset(&t);
    detail::serialize_impl(s, sr);
    return s;
}

} // json
} // boost

#endif
