//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_VALUE_FROM_HPP
#define BOOST_JSON_DETAIL_VALUE_FROM_HPP

#include <boost/json/storage_ptr.hpp>
#include <boost/json/value.hpp>
#include <boost/json/detail/value_traits.hpp>

namespace boost {
namespace json {

struct value_from_tag { };

template<class T, class = void>
struct has_value_from;

namespace detail {

template<std::size_t Max, std::size_t I = 0, class T>
void
tuple_to_array(T&& t, array& a)
{
    // KRYSTIAN TODO: use ADL and class members here,
    // forward the element based on Tuple
    using std::get;
    a.emplace(a.begin() + I, value_from(
        get<I>(std::forward<T>(t)), a.storage()));
    if (I >= Max - 1)
        return;
    // this prevents infinite recursive instantiation
    return detail::tuple_to_array<Max, 
        I + (I < Max - 1)>(
            std::forward<T>(t), a);
}

//----------------------------------------------------------

// Matches types from which value can be constructed
template<class T, typename std::enable_if<
    std::is_assignable<value&, T&&>::value>::type* = nullptr>
void
tag_invoke(
    value_from_tag,
    value& jv,
    T&& from)
{
    jv = std::forward<T>(from);
}

//----------------------------------------------------------
// Generic conversions

// tuple-like types
template<class T, typename std::enable_if<
    (std::tuple_size<remove_cvref<T>>::value > 0)>::type* = nullptr>
void
value_from_generic(
    value& jv,
    T&& from,
    priority_tag<2>)
{
    constexpr std::size_t elements =
        std::tuple_size<remove_cvref<T>>::value;
    array& arr = jv.emplace_array();
    arr.reserve(elements);
    detail::tuple_to_array<elements>(std::forward<T>(from), arr);
}

// map-like types
template<class T, typename std::enable_if<
    map_traits<T>::has_unique_keys && 
        has_value_from<typename map_traits<T>::pair_value_type>::value &&
    std::is_convertible<typename map_traits<T>::pair_key_type, 
        string_view>::value>::type* = nullptr>
void
value_from_generic(
    value& jv,
    T&& from,
    priority_tag<1>)
{
    using std::get;
    object& obj = jv.emplace_object();
    obj.reserve(container_traits<T>::try_size(from));
    for (auto&& elem : from)
        obj.emplace(get<0>(elem), value_from(
            get<1>(elem), obj.storage()));
}

// all other containers
template<class T, typename std::enable_if<
    has_value_from<typename container_traits<T>::
        value_type>::value>::type* = nullptr>
void
value_from_generic(
    value& jv,
    T&& from,
    priority_tag<0>)
{
    array& result = jv.emplace_array();
    result.reserve(container_traits<T>::try_size(from));
    for (auto&& elem : from)
        result.emplace_back(
            value_from(elem, result.storage()));
}

template<class T, void_t<typename std::enable_if<
    !std::is_assignable<value&, T&&>::value>::type,
        decltype(detail::value_from_generic(std::declval<value&>(), 
            std::declval<T&&>(), priority_tag<2>()))>* = nullptr>
void
tag_invoke(
    value_from_tag,
    value& jv,
    T&& from)
{
    detail::value_from_generic(jv,
        std::forward<T>(from), priority_tag<2>());
}

//----------------------------------------------------------

// Calls to value_from are forwarded to this function
// so we can use ADL and hide the built-in tag_invoke
// overloads in the detail namespace
template<class T, void_t<
    decltype(tag_invoke(std::declval<value_from_tag&>(),
        std::declval<value&>(), std::declval<T&&>()))>* = nullptr>
value
value_from_impl(
    T&& from,
    storage_ptr sp)
{
    value jv(detail::move(sp));
    tag_invoke(value_from_tag(), jv, std::forward<T>(from));
    return jv;
}

} // detail
} // json
} // boost

#endif
