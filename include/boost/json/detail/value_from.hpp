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

#include <boost/json/value.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/json/detail/value_conversion.hpp>
#include <type_traits>

namespace boost {
namespace json {

struct value_from_tag { };

template<class T, class = void>
struct has_value_from;

namespace detail {

//----------------------------------------------------------
// Generic constraints

template<std::size_t Max, std::size_t Idx = 0, typename T>
array&
tuple_to_array(T&& tup, array& input)
{
    // KRYSTIAN TODO: use ADL and class members here,
    // forward the element based on Tuple
    using std::get;
    input.emplace(input.begin() + Idx, value_from(
        get<Idx>(std::forward<T>(tup)), input.storage()));
    if (Idx >= Max - 1)
        return input;
    // this prevents infinite recursive instantiation
    return detail::tuple_to_array<Max, 
        Idx + (Idx < Max - 1)>(
            std::forward<T>(tup), input);
}

//----------------------------------------------------------
// Use one of value's constructors

template<typename T>
struct is_init_list : std::false_type { };

template<typename T>
struct is_init_list<std::initializer_list<T>> : std::true_type { };

template<typename T, typename U>
using no_cvref_same = std::is_same<remove_cvref<T>, U>;

template<typename T, typename std::enable_if<
    std::is_arithmetic<remove_cvref<T>>::value || no_cvref_same<T, value>::value ||
        no_cvref_same<T, std::nullptr_t>::value || no_cvref_same<T, object>::value ||
    no_cvref_same<T, array>::value || no_cvref_same<T, string>::value ||
        std::is_convertible<remove_cvref<T>, string_view>::value || 
    no_cvref_same<T, value_ref>::value || is_init_list<remove_cvref<T>>::
        value>::type* = nullptr>
value
value_from_impl(
    T&& from,
    storage_ptr sp,
    priority_tag<16>)
{
    return value(std::forward<T>(from), move(sp));
}

//----------------------------------------------------------
// Use customization point

template<typename T, void_t<
    decltype(tag_invoke(std::declval<value_from_tag&>(),
        std::declval<value&>(), std::declval<T&&>()))>* = nullptr>
value
value_from_impl(
    T&& from,
    storage_ptr sp,
    priority_tag<15>)
{
    value result(move(sp));
    tag_invoke(value_from_tag(), result, std::forward<T>(from));
    return result;
}

//----------------------------------------------------------
// Use generic conversion

// Matches tuples
// KRYSTIAN TODO: check if all element types
// can be converted to value with value_from
template<typename T, void_t<decltype(
    std::tuple_size<remove_cvref<T>>::value)>* = nullptr>
value
value_from_impl(
    T&& from,
    storage_ptr sp,
    priority_tag<14>)
{
    constexpr std::size_t elements = 
        std::tuple_size<remove_cvref<T>>::value;
    array result(move(sp));
    result.reserve(elements);
    return move(detail::tuple_to_array<elements>(
        std::forward<T>(from), result));
}

// Matches containers that can be converted to json::object
template<typename T, typename std::enable_if<
    map_traits<T>::has_unique_keys && has_value_from<typename
        map_traits<T>::pair_value_type>::value && 
    map_traits<T>::key_converts_to_string>::type* = nullptr>
value
value_from_impl(
    T&& from,
    storage_ptr sp,
    priority_tag<14>)
{
    using std::get;
    object result(move(sp));
    result.reserve(container_traits<T>::try_size(from));
    for (auto&& elem : from)
        result.emplace(get<0>(elem), value_from(
            get<1>(elem), result.storage()));
    return result;
}

// Matches ForwardRange
template<typename T, typename std::enable_if<
    has_value_from<typename container_traits<T>::
        value_type>::value>::type* = nullptr>
value
value_from_impl(
    T&& from,
    storage_ptr sp,
    priority_tag<13>)
{
    array result(move(sp));
    result.reserve(container_traits<T>::try_size(from));
    for (auto&& elem : from)
        result.emplace_back(value_from(elem, result.storage()));
    return result;
}

} // detail
} // json
} // boost

#endif
