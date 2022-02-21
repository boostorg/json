//
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
// Copyright (c) 2022 Dmitry Arkhipov (grisumbras@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_VALUE_TRAITS_HPP
#define BOOST_JSON_DETAIL_VALUE_TRAITS_HPP

#include <boost/json/detail/config.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/bind.hpp>
#include <boost/mp11/function.hpp>
#include <boost/mp11/utility.hpp>
#include <type_traits>
#include <utility>

BOOST_JSON_NS_BEGIN

struct value_from_tag { };

template<class>
struct value_to_tag { };

namespace detail {

using std::begin;
using std::end;
#ifdef __cpp_lib_nonmember_container_access
using std::size;
#endif

template<std::size_t I, class T>
using tuple_element_t = typename std::tuple_element<I, T>::type;

template<class T>
using value_type = remove_cvref< decltype(*begin(std::declval<T&>())) >;
template<class T>
using mapped_type = tuple_element_t< 1, value_type<T> >;

// had to make the metafunction always succeeding in order to make it work
// with msvc 14.0
template<class T>
using key_type_helper = tuple_element_t< 0, value_type<T> >;
template<class T>
using key_type = mp11::mp_eval_or<
    void,
    key_type_helper,
    T>;

template<class T>
using has_size_member_helper
    = std::is_convertible<decltype(std::declval<T&>().size()), std::size_t>;
template<class T>
using has_size_member = mp11::mp_valid_and_true<has_size_member_helper, T>;;
template<class T>
using has_free_size_helper
    = std::is_convertible<
        decltype(size(std::declval<T const&>())),
        std::size_t>;
template<class T>
using has_free_size = mp11::mp_valid_and_true<has_free_size_helper, T>;;
template<class T>
using size_implementation = mp11::mp_cond<
    has_size_member<T>, mp11::mp_int<3>,
    has_free_size<T>,   mp11::mp_int<2>,
    std::is_array<T>,   mp11::mp_int<1>,
    mp11::mp_true,      mp11::mp_int<0>>;

template<class T>
std::size_t
try_size(T&& cont, mp11::mp_int<3>)
{
    return cont.size();
}

template<class T>
std::size_t
try_size(T& cont, mp11::mp_int<2>)
{
    return size(cont);
}

template<class T, std::size_t N>
std::size_t
try_size(T(&)[N], mp11::mp_int<1>)
{
    return N;
}

template<class T>
std::size_t
try_size(T&, mp11::mp_int<0>)
{
    return 0;
}

using value_from_conversion = mp11::mp_true;
using value_to_conversion = mp11::mp_false;

struct user_conversion_tag { };
struct native_conversion_tag { };
struct value_conversion_tag : native_conversion_tag { };
struct object_conversion_tag : native_conversion_tag { };
struct array_conversion_tag : native_conversion_tag { };
struct string_conversion_tag : native_conversion_tag { };
struct bool_conversion_tag : native_conversion_tag { };
struct number_conversion_tag : native_conversion_tag { };
struct nullptr_conversion_tag : native_conversion_tag { };
struct string_like_conversion_tag { };
struct map_like_conversion_tag { };
struct sequence_conversion_tag { };
struct tuple_conversion_tag { };
struct no_conversion_tag { };

template<class T>
using has_user_conversion_from_impl
    = decltype(tag_invoke(
        value_from_tag(), std::declval<value&>(), std::declval<T&&>()));
template<class T>
using has_user_conversion_to_impl
    = decltype(tag_invoke(
        std::declval<value_to_tag<T>&>(), std::declval<value const &>()));
template<class T, class Dir>
using has_user_conversion
    = mp11::mp_valid_q<
        mp11::mp_if<
            std::is_same<Dir, value_from_conversion>,
            mp11::mp_quote<has_user_conversion_from_impl>,
            mp11::mp_quote<has_user_conversion_to_impl>>,
        T>;

template<class T, class U>
using is_data_convertible
    = std::is_convertible<decltype(std::declval<T&>().data()), U>;
template<class T>
using is_string_like = mp11::mp_all<
    std::is_constructible<T, char const*, std::size_t>,
    mp11::mp_valid_and_true<is_data_convertible, T, char const*>,
    mp11::mp_valid_and_true<has_size_member_helper, T>>;

template<class T>
using are_begin_and_end_same = std::is_same<
    decltype(begin(std::declval<T&>())),
    decltype(end(std::declval<T&>()))>;
template <class T>
using is_sequence_like = mp11::mp_valid_and_true<are_begin_and_end_same, T>;

template<class T>
using has_positive_tuple_size = mp11::mp_bool<
    (std::tuple_size<T>::value > 0) >;
template<class T>
using is_tuple_like = mp11::mp_valid_and_true<has_positive_tuple_size, T>;

template<class T>
using has_unique_keys = has_positive_tuple_size<decltype(
    std::declval<T&>().emplace(
        std::declval<value_type<T>>()))>;
template<class T>
using is_value_type_pair
    = mp11::mp_bool<std::tuple_size<value_type<T>>::value == 2>;
template<class T, class Dir>
using is_map_like = mp11::mp_all<
    is_sequence_like<T>,
    mp11::mp_valid_and_true<has_unique_keys, T>,
    mp11::mp_valid_and_true<is_value_type_pair, T>,
    mp11::mp_invoke_q<
        mp11::mp_if<
            Dir, // is value_from_conversion
            mp11::mp_quote<std::is_convertible>,
            mp11::mp_quote<std::is_constructible>>,
        key_type<T>,
        string_view>>;

// matches char const*, but only for value_from
template<class T, class Dir>
using is_char_star
    = mp11::mp_and<Dir, std::is_same<T, char const*>>;

template<class T, class Dir>
using conversion_implementation = mp11::mp_cond<
    // user conversion (via tag_invoke)
    has_user_conversion<T, Dir>,       user_conversion_tag,
    // native conversions (constructors and member functions of value)
    std::is_same<T, value>,            value_conversion_tag,
    std::is_same<T, array>,            array_conversion_tag,
    std::is_same<T, object>,           object_conversion_tag,
    std::is_same<T, string>,           string_conversion_tag,
    std::is_same<T, std::nullptr_t>,   nullptr_conversion_tag,
    std::is_same<T, bool>,             bool_conversion_tag,
    std::is_arithmetic<T>,             number_conversion_tag,
    is_char_star<T, Dir>,              native_conversion_tag,
    // generic conversions
    is_string_like<T>,                 string_like_conversion_tag,
    is_map_like<T, Dir>,               map_like_conversion_tag,
    is_sequence_like<T>,               sequence_conversion_tag,
    is_tuple_like<T>,                  tuple_conversion_tag,
    // failed to find a suitable implementation
    mp11::mp_true,                     no_conversion_tag>;

template <class T, class Dir>
using can_convert = mp11::mp_not<
    std::is_same<
        detail::conversion_implementation<T, Dir>,
        detail::no_conversion_tag>>;

template<class T>
using value_from_implementation
    = conversion_implementation<T, value_from_conversion>;

template<class T>
using value_to_implementation
    = conversion_implementation<T, value_to_conversion>;

template<class Main, class Opposite>
using conversion_round_trips_helper = mp11::mp_or<
    std::is_same<Main, Opposite>,
    std::is_same<user_conversion_tag, Main>,
    std::is_same<user_conversion_tag, Opposite>,
    std::is_same<no_conversion_tag, Opposite>>;
template<class T, class Dir>
using conversion_round_trips  = conversion_round_trips_helper<
    conversion_implementation<T, Dir>,
    conversion_implementation<T, mp11::mp_not<Dir>>>;

} // detail
BOOST_JSON_NS_END

#endif
