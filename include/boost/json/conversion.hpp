//
// Copyright (c) 2022 Dmitry Arkhipov (grisumbras@yandex.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_CONVERSION_HPP
#define BOOST_JSON_CONVERSION_HPP

#include <boost/json/string_view.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/utility.hpp>

#include <utility>

BOOST_JSON_NS_BEGIN
namespace detail {

using std::begin;
using std::end;

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
using are_begin_and_end_same = std::is_same<
    decltype(begin(std::declval<T&>())),
    decltype(end(std::declval<T&>()))>;

template<class T>
using has_positive_tuple_size = mp11::mp_bool<
    (std::tuple_size<T>::value > 0) >;

template<class T>
using has_unique_keys = has_positive_tuple_size<decltype(
    std::declval<T&>().emplace(
        std::declval<value_type<T>>()))>;

template<class T>
using is_value_type_pair
    = mp11::mp_bool<std::tuple_size<value_type<T>>::value == 2>;

} // namespace detail

/** Determine if `T` can be treated like a string during conversions.

    Provides the member constant `value` that is equal to `true`, if `T` is
    convertible to @ref string_view. Otherwise, `value` is equal to `false`.
    <br>

    Users can specialize the trait for their own types if they don't want them
    to be treated like strings. For example:

    @code
    namespace boost {
    namespace json {

    template <>
    struct is_string_like<your::string> : std::false_type
    { };

    } // namespace boost
    } // namespace json
    @endcode

    @par Types satisfying the trait

    @ref string,
    @ref string_view,
    <a href="https://en.cppreference.com/w/cpp/string/basic_string"><tt>std::string</tt></a>,
    <a href="https://en.cppreference.com/w/cpp/string/basic_string_view"><tt>std::string_view</tt></a>.

    @see @ref value_from, @ref value_to
*/
template<class T, class Enable = void>
struct is_string_like
#ifndef BOOST_JSON_DOCS
    : std::is_convertible<T, string_view>
{ };
#else
;
#endif

/** Determine if `T` can be treated like a sequence during conversions.

    Given `t`, a glvalue of type `T`, if
    <tt>std::is_same<decltype(BEGIN(t)), decltype(END(t))>::value</tt> is
    well-formed and `true`, then the trait provides the member constant `value`
    that is equal to `true`. Otherwise, `value` is equal to `false`.<br>

    Operation `BEGIN(t)` is equivalent to

    @code
    using std::begin;
    begin(t);
    @endcode

    Operation `END(t)` is equivalent to

    @code
    using std::end;
    end(t);
    @endcode

    Users can specialize the trait for their own types if they don't want them
    to be treated like sequences. For example:

    @code
    namespace boost {
    namespace json {

    template <>
    struct is_sequence_like<your::container> : std::false_type
    { };

    } // namespace boost
    } // namespace json
    @endcode


    @par Types satisfying the trait

    Any <a href="https://en.cppreference.com/w/cpp/named_req/SequenceContainer"><em>SequenceContainer</em></a>,
    array types.

    @see @ref value_from, @ref value_to
*/
template<class T, class Enable = void>
struct is_sequence_like
#ifndef BOOST_JSON_DOCS
    : mp11::mp_valid_and_true<detail::are_begin_and_end_same, T>
{ };
#else
;
#endif

/** Determine if `T` can be treated like a 1-to-1 mapping during
    conversions.

    Given `t`, a glvalue of type `T`, if

    @li <tt>is_sequence_like<T>::value</tt> is `true`; and

    @li given `V`, the type denoted by
        `std::remove_cvref_t<decltype(*BEGIN(t))>`,
        <tt>std::tuple_size<V>::value</tt> is equal to 2; and

    @li given `K`, the type denoted by `std::tuple_element_t<0, V>`,
        <tt>std::is_string_like<K>::value</tt> is `true`; and

    @li given `v`, a glvalue of type `V`, and `E`, the type denoted by
        `decltype(t.emplace(v))`,
        <tt>std::is_tuple_like<E>::value</tt> is `true`;

    then the trait provides the member constant `value`
    that is equal to `true`. Otherwise, `value` is equal to `false`.<br>

    Operation `BEGIN(t)` is equivalent to

    @code
    using std::begin;
    begin(t);
    @endcode

    Users can specialize the trait for their own types if they don't want them
    to be treated like mappings. For example:

    @code
    namespace boost {
    namespace json {

    template <>
    struct is_map_like<your::map> : std::false_type
    { };

    } // namespace boost
    } // namespace json
    @endcode


    @par Types satisfying the trait

    <a href="https://en.cppreference.com/w/cpp/container/map"><tt>std::map</tt></a>,
    <a href="https://en.cppreference.com/w/cpp/container/unordered_map"><tt>std::unordered_map</tt></a>.

    @see @ref value_from, @ref value_to
*/
template<class T, class Enable = void>
struct is_map_like
#ifndef BOOST_JSON_DOCS
    : mp11::mp_all<
        is_sequence_like<T>,
        is_string_like<detail::key_type<T>>,
        mp11::mp_valid_and_true<detail::has_unique_keys, T>,
        mp11::mp_valid_and_true<detail::is_value_type_pair, T>>
{ };
#else
;
#endif

/** Determine if `T` can be treated like a tuple during conversions.

    Provides the member constant `value` that is equal to `true`, if
    <tt>std::tuple_size<T>::value</tt> is a positive number. Otherwise, `value`
    is equal to `false`.<br>

    Users can specialize the trait for their own types if they don't want them
    to be treated like tuples. For example:

    @code
    namespace boost {
    namespace json {

    template <>
    struct is_tuple_like<your::tuple> : std::false_type
    { };

    } // namespace boost
    } // namespace json
    @endcode


    @par Types satisfying the trait

    <a href="https://en.cppreference.com/w/cpp/utility/tuple"><tt>std::tuple</tt></a>,
    <a href="https://en.cppreference.com/w/cpp/utility/pair"><tt>std::pair</tt></a>.

    @see @ref value_from, @ref value_to
*/
template<class T, class Enable = void>
struct is_tuple_like
#ifndef BOOST_JSON_DOCS
    : mp11::mp_valid_and_true<detail::has_positive_tuple_size, T>
{ };
#else
;
#endif

BOOST_JSON_NS_END

#endif // BOOST_JSON_CONVERSION_HPP
