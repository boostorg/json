//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_MAKE_VOID_HPP
#define BOOST_JSON_DETAIL_MAKE_VOID_HPP

namespace boost {
namespace json {
namespace detail {

template<class...>
struct make_void
{
    using type =void;
};

template<class... Ts>
using void_t = typename
    make_void<Ts...>::type;

} // detail
} // json
} // boost

#endif
