//
// Copyright (c) 2022 Dmitry Arkhipov (grisumbras@yandex.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_CONVERSION_HPP
#define BOOST_JSON_IMPL_CONVERSION_HPP

#include <boost/json/value.hpp>
#include <boost/json/string_view.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/utility.hpp>

#include <utility>
#ifndef BOOST_NO_CXX17_HDR_VARIANT
# include <variant>
#endif // BOOST_NO_CXX17_HDR_VARIANT

BOOST_JSON_NS_BEGIN
namespace detail {

template<std::size_t I, class T>
using tuple_element_t = typename std::tuple_element<I, T>::type;

template<class T>
using value_type = typename std::iterator_traits<decltype(
    std::begin(std::declval<T&>()) )>::value_type;
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
using iterator_type = decltype(std::begin(std::declval<T&>()));
template<class T>
using are_begin_and_end_same = std::is_same<
    iterator_type<T>,
    decltype(std::end(std::declval<T&>()))>;

template<class T>
using begin_iterator_category = typename std::iterator_traits<
    iterator_type<T>>::iterator_category;

template<class T>
using has_positive_tuple_size = mp11::mp_bool<
    (std::tuple_size<T>::value > 0) >;

template<class T>
using has_unique_keys = has_positive_tuple_size<decltype(
    std::declval<T&>().emplace(
        std::declval<value_type<T>>()))>;

template<class T>
struct is_value_type_pair_helper : std::false_type
{ };
template<class T1, class T2>
struct is_value_type_pair_helper<std::pair<T1, T2>> : std::true_type
{ };
template<class T>
using is_value_type_pair = is_value_type_pair_helper<value_type<T>>;

} // namespace detail

template <class T>
struct result_for<T, value>
{
    using type = result< detail::remove_cvref<T> >;
};

template<class T>
struct is_string_like
    : std::is_convertible<T, string_view>
{ };

template<class T>
struct is_sequence_like
    : mp11::mp_all<
        mp11::mp_valid_and_true<detail::are_begin_and_end_same, T>,
        mp11::mp_valid<detail::begin_iterator_category, T>>
{ };

template<class T>
struct is_map_like
    : mp11::mp_all<
        is_sequence_like<T>,
        mp11::mp_valid_and_true<detail::is_value_type_pair, T>,
        is_string_like<detail::key_type<T>>,
        mp11::mp_valid_and_true<detail::has_unique_keys, T>>
{ };

template<class T>
struct is_tuple_like
    : mp11::mp_valid_and_true<detail::has_positive_tuple_size, T>
{ };

template<>
struct is_null_like<std::nullptr_t>
    : std::true_type
{ };

#ifndef BOOST_NO_CXX17_HDR_VARIANT
template<>
struct is_null_like<std::monostate>
    : std::true_type
{ };
#endif // BOOST_NO_CXX17_HDR_VARIANT

BOOST_JSON_NS_END

#endif // BOOST_JSON_IMPL_CONVERSION_HPP
