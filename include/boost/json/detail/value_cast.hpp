//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_VALUE_CAST_HPP
#define BOOST_JSON_DETAIL_VALUE_CAST_HPP

#include <boost/json/number_cast.hpp>
#include <boost/json/traits.hpp>
#include <boost/json/value.hpp>
#include <type_traits>

namespace boost {
namespace json {
namespace detail {

template<class T, class = void>
struct has_value_cast_traits : std::false_type
{
};

template<class T>
struct has_value_cast_traits<
    T, void_t<decltype(
    value_cast_traits<T>::construct(
        std::declval<value const&>())
    ) > > : std::true_type
{
};

//----------------------------------------------------------

// generic representations

// contiguous container of char,
// e.g. std::string
template<class T>
T
value_cast_impl_3(
    value const& jv,
    typename std::integral_constant<bool,
        std::is_same<char*, decltype(
            std::declval<T>().data())>::value &&
        std::is_integral<decltype(
            std::declval<T const>().size())>::value &&
        std::is_constructible<
            T, char*, std::size_t>::value
    >::type* = 0)
{
    auto const& a = jv.as_string();
    return T(a.data(), a.size());
}

//----------------------------------------------------------

// direct representations

// object
template<class T>
object const&
value_cast_direct_impl(
    value const& jv,
    typename std::enable_if<
        std::is_same<T, object>::value
    >::type* = 0)
{
    return jv.as_object();
}

// array
template<class T>
array const&
value_cast_direct_impl(
    value const& jv,
    typename std::enable_if<
        std::is_same<T, array>::value
    >::type* = 0)
{
    return jv.as_array();
}

// string
template<class T>
string const&
value_cast_direct_impl(
    value const& jv,
    typename std::enable_if<
        std::is_same<T, string>::value
    >::type* = 0)
{
    return jv.as_string();
}

// numeric
template<class T>
T
value_cast_direct_impl(
    value const& jv,
    typename std::enable_if<
        ! std::is_same<T, bool>::value && (
        std::is_integral<T>::value ||
        std::is_floating_point<T>::value)
    >::type* = 0)
{
    return number_cast<T>(jv);
}

// bool
template<class T>
bool
value_cast_direct_impl(
    value const& jv,
    typename std::enable_if<
        std::is_same<T, bool>::value
    >::type* = 0)
{
    return jv.as_bool();
}

template<class T, class = void>
struct has_direct_impl : std::false_type
{
};

template<class T>
struct has_direct_impl<T, void_t<decltype(
    value_cast_direct_impl<T>(
        std::declval<value const&>())
    )>> : std::true_type
{
};

template<class T>
auto
value_cast_impl_2(
    value const& jv,
    std::true_type) -> decltype(
        value_cast_direct_impl<T>(jv))
{
    // use direct representation
    return value_cast_direct_impl<T>(jv);
}

template<class T>
auto
value_cast_impl_2(
    value const& jv,
    std::false_type) -> decltype(
        value_cast_impl_3<T>(jv))
{
    return value_cast_impl_3<T>(jv);
}

//----------------------------------------------------------

// traits

template<class T>
auto
value_cast_impl_1(
    value const& jv,
    std::true_type) -> decltype(
        value_cast_traits<T>::construct(jv))
{
    // use traits
    return value_cast_traits<T>::construct(jv);
}

template<class T>
auto
value_cast_impl_1(
    value const& jv,
    std::false_type) -> decltype(
        value_cast_impl_2<T>(jv,
        has_direct_impl<T>{}))
{
    return value_cast_impl_2<T>(jv,
        has_direct_impl<T>{});
}

} // detail
} // json
} // boost

#endif
