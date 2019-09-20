//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_VALUE_HPP
#define BOOST_JSON_DETAIL_VALUE_HPP

#include <boost/json/error.hpp>
#include <boost/type_traits/make_void.hpp>
#include <type_traits>

namespace boost {
namespace json {

class value;

namespace detail {

// Determine if `to_json(value&,T)` is found via ADL
template<class T, class = void>
struct has_adl_to_json : std::false_type
{
};

template<class T>
struct has_adl_to_json<T,
    boost::void_t<decltype(to_json(
        std::declval<T const&>(),
        std::declval<json::value&>()))>>
    : std::true_type
{
};

// Determine if `from_json(T&,value)` is found via ADL
template<class T, class = void>
struct has_adl_from_json : std::false_type
{
};

template<class T>
struct has_adl_from_json<T,
    boost::void_t<decltype(from_json(
        std::declval<T&>(),
        std::declval<json::value const&>()))>>
    : std::true_type
{
};

// Determine if `t.to_json(value&)` exists
template<class T, class = void>
struct has_mf_to_json : std::false_type
{
};

template<class T>
struct has_mf_to_json<T,
    boost::void_t<decltype(
        std::declval<T const&>().to_json(
            std::declval<json::value&>()))>>
    : std::true_type
{
};

// Determine if `t.to_json(value const&)` exists
template<class T, class = void>
struct has_mf_from_json : std::false_type
{
};

template<class T>
struct has_mf_from_json<T,
    boost::void_t<decltype(
    std::declval<T&>().from_json(
        std::declval<json::value const&>()))>>
    : std::true_type
{
};

template<class T>
void
call_to_json(
    T const& t,
    value& v,
    std::true_type)
{
    t.to_json(v);
}

template<class T>
void
call_to_json(
    T const& t,
    value& v,
    std::false_type)
{
    to_json(t, v);
}

template<class T>
void
call_to_json(
    T const& t,
    value& v)
{
    call_to_json(t, v,
        has_mf_to_json<T>{});
}

template<class T>
void
call_from_json(
    T& t,
    value const& v,
    std::true_type)
{
    t.from_json(v);
}

template<class T>
void
call_from_json(
    T& t,
    value const& v,
    std::false_type)
{
    from_json(t, v);
}

template<class T>
void
call_from_json(
    T& t,
    value const& v)
{
    call_from_json(t, v,
        has_mf_from_json<T>{});
}

} // detail
} // json
} // boost

#endif
