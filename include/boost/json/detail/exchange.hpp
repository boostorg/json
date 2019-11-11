//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_EXCHANGE_HPP
#define BOOST_JSON_DETAIL_EXCHANGE_HPP

#include <utility>

namespace boost {
namespace json {
namespace detail {

template<class T, class U>
inline
T
exchange(T& t, U u) noexcept
{
    T v = std::move(t);
    t = std::move(u);
    return v;
}

} // detail
} // json
} // boost

#endif
