//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_VALUE_TO_HPP
#define BOOST_JSON_DETAIL_VALUE_TO_HPP

#include <boost/json/number_cast.hpp>
#include <boost/json/value.hpp>
#include <boost/json/detail/value_conversion.hpp>

#include <type_traits>

namespace boost {
namespace json {

template<class>
struct value_to_tag { };

template<class T, class = void>
struct has_value_to;

namespace detail {

//----------------------------------------------------------
// Use native conversion

// identity conversion
inline
value
value_to_impl(
    value const& jv,
    value_to_tag<value>,
    priority_tag<16>)
{
    return jv;
}

// object
inline
object
value_to_impl(
    value const& jv,
    value_to_tag<object>,
    priority_tag<16>)
{
    return jv.as_object();
}

// array
inline
array
value_to_impl(
    value const& jv,
    value_to_tag<array>,
    priority_tag<16>)
{
    return jv.as_array();
}

// string
inline
string
value_to_impl(
    value const& jv,
    value_to_tag<string>,
    priority_tag<16>)
{
    return jv.as_string();
}

// bool
inline
bool
value_to_impl(
    value const& jv,
    value_to_tag<bool>,
    priority_tag<16>)
{
    return jv.as_bool();
}

// integral and floating point
template<typename T, typename std::enable_if<
    std::is_arithmetic<T>::value>::type* = nullptr>
T
value_to_impl(
    value const& jv,
    value_to_tag<T>,
    priority_tag<16>)
{
    return number_cast<T>(jv);
}

//----------------------------------------------------------
// Use customization point

template<typename T>
auto
value_to_impl(
    value const& jv,
    value_to_tag<T> tag,
    priority_tag<15>) ->
        decltype(tag_invoke(tag, jv))
{
    return tag_invoke(tag, jv);
}

//----------------------------------------------------------
// Use one of T's constructors

template<typename T>
auto
value_to_impl(
    value const& jv,
    value_to_tag<T>,
    priority_tag<14>) ->
        decltype(T(jv))
{
    return T(jv);
}

//----------------------------------------------------------
// Use generic conversion

// Matches string-like types
template<typename T, typename std::enable_if<
    std::is_constructible<T, char*, std::size_t>::value &&
        std::is_convertible<char*, decltype(
    std::declval<T&>().data())>::value &&
        std::is_convertible<decltype(std::declval<T&>().size()),
    std::size_t>::value>::type* = nullptr>
T
value_to_impl(
    value const& jv,
    value_to_tag<T>,
    priority_tag<13>)
{
    auto& str = jv.as_string();
    return T(str.data(), str.size());
}

// Matches containers that can be constructed from json::object
template<typename T, typename std::enable_if<
    has_value_to<typename map_traits<T>::pair_value_type>::value &&
        std::is_constructible<string_view, typename
    map_traits<T>::pair_key_type>::value>::type* = nullptr>
T
value_to_impl(
    value const& jv,
    value_to_tag<T>,
    priority_tag<12>)
{
    using value_type = typename
        container_traits<T>::value_type;
    const object& obj = jv.as_object();
    T result;
    container_traits<T>::try_reserve(
        result, obj.size());
    for (const auto& val : obj)
        result.insert(value_type{typename map_traits<T>::
            pair_key_type(val.key()), value_to_impl(val.value(), 
                value_to_tag<typename map_traits<T>::pair_value_type>(), 
                    priority_tag<16>())});
    return result;
}

// Matches ForwardRange
template<typename T, typename std::enable_if<
    has_value_to<typename container_traits<T>::
        value_type>::value>::type* = nullptr>
T
value_to_impl(
    value const& jv,
    value_to_tag<T>,
    priority_tag<11>)
{
    const array& arr = jv.as_array();
    T result;
    container_traits<T>::try_reserve(
        result, arr.size());
    for (const auto& val : arr)
        result.insert(end(result), value_to_impl(val, 
            value_to_tag<typename container_traits<T>::value_type>(),
                priority_tag<16>()));
    return result;
}

//----------------------------------------------------------


} // detail
} // json
} // boost

#endif
