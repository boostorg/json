//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
// Copyright (c) 2021 Dmitry Arkhipov (grisumbras@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_VALUE_TO_HPP
#define BOOST_JSON_DETAIL_VALUE_TO_HPP

#include <boost/json/value.hpp>
#include <boost/json/detail/value_traits.hpp>
#include <boost/mp11/integer_sequence.hpp>

#include <type_traits>

BOOST_JSON_NS_BEGIN

template<class T, class U,
    typename std::enable_if<
        ! std::is_reference<T>::value &&
    std::is_same<U, value>::value>::type>
T value_to(U const&);

namespace detail {

template<class T>
using has_reserve_member_helper = decltype(std::declval<T&>().reserve(0));
template<class T>
using has_reserve_member = mp11::mp_valid<has_reserve_member_helper, T>;
template<class T>
using reserve_implementation = mp11::mp_cond<
    is_tuple_like<T>,      mp11::mp_int<2>,
    has_reserve_member<T>, mp11::mp_int<1>,
    mp11::mp_true,         mp11::mp_int<0>>;

template<class T>
void
try_reserve(
    T&,
    std::size_t size,
    mp11::mp_int<2>)
{
    constexpr std::size_t N = std::tuple_size<remove_cvref<T>>::value;
    if ( N != size )
    {
        detail::throw_invalid_argument(
            "target array size does not match source array size",
            BOOST_CURRENT_LOCATION);
    }
}

template<typename T>
void
try_reserve(
    T& cont,
    std::size_t size,
    mp11::mp_int<1>)
{
    cont.reserve(size);
}

template<typename T>
void
try_reserve(
    T&,
    std::size_t,
    mp11::mp_int<0>)
{
}


template<class T>
using has_push_back_helper
    = decltype(std::declval<T&>().push_back(std::declval<value_type<T>>()));
template<class T>
using has_push_back = mp11::mp_valid<has_push_back_helper, T>;
template<class T>
using inserter_implementation = mp11::mp_cond<
    is_tuple_like<T>, mp11::mp_int<2>,
    has_push_back<T>, mp11::mp_int<1>,
    mp11::mp_true,    mp11::mp_int<0>>;

template<class T>
iterator_type<T>
inserter(
    T& target,
    mp11::mp_int<2>)
{
    return target.begin();
}

template<class T>
std::back_insert_iterator<T>
inserter(
    T& target,
    mp11::mp_int<1>)
{
    return std::back_inserter(target);
}

template<class T>
std::insert_iterator<T>
inserter(
    T& target,
    mp11::mp_int<0>)
{
    return std::inserter(target, end(target));
}

// identity conversion
inline
value
value_to_impl(
    value_to_tag<value>,
    value const& jv,
    value_conversion_tag)
{
    return jv;
}

// object
inline
object
value_to_impl(
    value_to_tag<object>,
    value const& jv,
    object_conversion_tag)
{
    return jv.as_object();
}

// array
inline
array
value_to_impl(
    value_to_tag<array>,
    value const& jv,
    array_conversion_tag)
{
    return jv.as_array();
}

// string
inline
string
value_to_impl(
    value_to_tag<string>,
    value const& jv,
    string_conversion_tag)
{
    return jv.as_string();
}

// bool
inline
bool
value_to_impl(
    value_to_tag<bool>,
    value const& jv,
    bool_conversion_tag)
{
    return jv.as_bool();
}

// integral and floating point
template<class T>
T
value_to_impl(
    value_to_tag<T>,
    value const& jv,
    number_conversion_tag)
{
    return jv.to_number<T>();
}

// null-like conversion
template<class T>
T
value_to_impl(
    value_to_tag<T>,
    value const& jv,
    null_like_conversion_tag)
{
    if( jv.is_null() )
        return T();
    detail::throw_invalid_argument(
        "source value is not null", BOOST_CURRENT_LOCATION);
}

// string-like types
template<class T>
T
value_to_impl(
    value_to_tag<T>,
    value const& jv,
    string_like_conversion_tag)
{
    auto& str = jv.as_string();
    return T(str.subview());
}

// map-like containers
template<class T>
T
value_to_impl(
    value_to_tag<T>,
    value const& jv,
    map_like_conversion_tag)
{
    const object& obj = jv.as_object();
    T result;
    detail::try_reserve(result, obj.size(), reserve_implementation<T>());
    std::transform(obj.begin(), obj.end(),
        detail::inserter(result, inserter_implementation<T>()),
        [](key_value_pair const& val) {
            return value_type<T>{
                key_type<T>(val.key()),
                value_to<mapped_type<T>>(val.value())};
        });
    return result;
}

// all other containers
template<class T>
T
value_to_impl(
    value_to_tag<T>,
    value const& jv,
    sequence_conversion_tag)
{
    array const& arr = jv.as_array();
    T result;
    detail::try_reserve(result, arr.size(), reserve_implementation<T>());
    using inserter_impl = inserter_implementation<T>;
    std::transform(arr.begin(), arr.end(),
        detail::inserter(result, inserter_impl()), [](value const& val) {
            return value_to<value_type<T>>(val);
        });
    return result;
}

template <class T, std::size_t... Is>
T
make_tuple_like(const array& arr, boost::mp11::index_sequence<Is...>)
{
    return T(value_to<typename std::tuple_element<Is, T>::type>(arr[Is])...);
}

// tuple-like types
template<class T>
T
value_to_impl(
    value_to_tag<T>,
    value const& jv,
    tuple_conversion_tag)
{
    auto& arr = jv.as_array();
    constexpr std::size_t N = std::tuple_size<remove_cvref<T>>::value;
    if ( N != arr.size() )
    {
        detail::throw_invalid_argument(
            "array size does not match tuple size",
            BOOST_CURRENT_LOCATION);
    }

    return make_tuple_like<T>(arr, boost::mp11::make_index_sequence<N>());
}

//----------------------------------------------------------
// User-provided conversion
template<class T>
T
value_to_impl(
    value_to_tag<T> tag,
    value const& jv,
    user_conversion_tag)
{
    return tag_invoke(tag, jv);
}

// no suitable conversion implementation
template<class T>
T
value_to_impl(
    value_to_tag<T>,
    value const&,
    no_conversion_tag)
{
    static_assert(
        !std::is_same<T, T>::value,
        "No suitable tag_invoke overload found for the type");
}

template<class T>
using value_to_implementation
    = conversion_implementation<T, value_to_conversion>;

} // detail
BOOST_JSON_NS_END

#endif
