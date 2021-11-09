//
// Copyright (c) 2021 Dmitry Arkhipov (grisumbras@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_INDEX_SEQUENCE_HPP
#define BOOST_JSON_DETAIL_INDEX_SEQUENCE_HPP

#include <boost/json/detail/config.hpp>
#include <boost/mp11/integer_sequence.hpp>

BOOST_JSON_NS_BEGIN
namespace detail {

template <std::size_t... Is>
using index_sequence = boost::mp11::index_sequence<Is...>;

template <std::size_t N>
using make_index_sequence = boost::mp11::make_index_sequence<N>;

} // detail
BOOST_JSON_NS_END

#endif // BOOST_JSON_DETAIL_INDEX_SEQUENCE_HPP
