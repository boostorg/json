//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_IS_SPECIALIZED_HPP
#define BOOST_JSON_DETAIL_IS_SPECIALIZED_HPP

#include <type_traits>

namespace boost {
namespace beast {
namespace json {
namespace detail {

struct primary_template
{
};

template<class T>
using is_specialized =
    std::integral_constant<bool,
        ! std::is_base_of<primary_template, T>::value>;

template<class T>
using remove_cr =
    typename std::remove_const<
    typename std::remove_reference<T>::type>::type;

} // detail
} // json
} // beast
} // boost

#endif
