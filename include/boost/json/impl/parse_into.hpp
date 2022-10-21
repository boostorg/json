//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_PARSE_INTO_HPP
#define BOOST_JSON_IMPL_PARSE_INTO_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/detail/parse_into.hpp>
#include <boost/json/error.hpp>

namespace boost {
namespace json {

template<class V>
void
parse_into(
    V& v,
    string_view sv,
    error_code& ec,
    parse_options const& opt )
{
    parser_for<V> p( opt, &v );

    std::size_t n = p.write_some( false, sv.data(), sv.size(), ec );

    if( !ec && n < sv.size() )
    {
        BOOST_JSON_FAIL( ec, error::extra_data );
    }
}

} // namespace boost
} // namespace json

#endif
