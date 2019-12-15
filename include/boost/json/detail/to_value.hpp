//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_TO_VALUE_HPP
#define BOOST_JSON_DETAIL_TO_VALUE_HPP

#include <boost/json/value.hpp>
#include <boost/json/storage_ptr.hpp>
#include <type_traits>

namespace boost {
namespace json {

template<class T, class = void>
struct has_to_value;

namespace detail {

template<class T, class = void>
struct has_to_value_traits : std::false_type
{
};

template<class T>
struct has_to_value_traits<T,
    detail::void_t<decltype(
        to_value_traits<T>::assign(
            std::declval<value&>(),
            std::declval<T&&>())
    )>> : std::true_type
{
};

template<class T, class = void>
struct has_to_json_mf : std::false_type
{
};

template<class T>
struct has_to_json_mf<T,
    detail::void_t<decltype(
        std::declval<T const&>().to_json(
            std::declval<value&>())
    )>> : std::true_type
{
};

//----------------------------------------------------------

#if 0
template<class T>
void
to_value_impl_4( ... )
{
    // Should never get here, because
    // `to_value` is constrained to known types.
    static_assert(sizeof(T)==0,
        "to_value called for an unknown type");
}
#endif

// generic algorithms

// ForwardRange
template<class T>
typename std::enable_if<
    has_to_value<decltype(
        *std::declval<T const&>().begin())>::value &&
    std::is_same<
        decltype(std::declval<T const&>().begin()),
        decltype(std::declval<T const&>().end())>::value
    >::type
to_value_impl_4(
    value& jv,
    T const& t)
{
    auto& a = jv.emplace_array();
    for(auto const& e : t)
        a.emplace_back(
            to_value(e, a.storage()));
}

// ContiguousContainer<char>
template<class T>
typename std::enable_if<
    std::is_same<char, decltype(
        *std::declval<T const&>().data())>::value &&
    std::is_convertible<decltype(
        std::declval<T const&>().size()),
        std::size_t>::value
    >::type
to_value_impl_4(
    value& jv,
    T const& t)
{
    jv = string_view(t.data(), t.size());
}

//----------------------------------------------------------

// These declarations must come after
// all the declarations of `to_value_impl_4`

template<class T, class = void>
struct has_to_value_generic : std::false_type
{
};

template<class T>
struct has_to_value_generic<T,
    detail::void_t<decltype(
        to_value_impl_4(
            std::declval<value&>(),
            std::declval<T const&>())
    )>> : std::true_type
{
};

//----------------------------------------------------------

// Use T::to_value
template<class T>
void
to_value_impl_3(
    value& jv,
    T&& t,
    std::true_type)
{
    return std::forward<T>(t).to_json(jv);
}

template<class T>
void
to_value_impl_3(
    value& jv,
    T&& t,
    std::false_type)
{
    to_value_impl_4(jv,
        std::forward<T>(t));
}

//----------------------------------------------------------

// Use to_value_traits
template<class T>
void
to_value_impl_2(
    value& jv,
    T&& t,
    std::true_type)
{
    return to_value_traits<
        remove_cvref<T>>::assign(jv,
            std::forward<T>(t));
}

template<class T>
void
to_value_impl_2(
    value& jv,
    T&& t,
    std::false_type)
{
    to_value_impl_3(jv,
        std::forward<T>(t),
        has_to_json_mf<
            remove_cvref<T>>{});
}

//----------------------------------------------------------

// Use value(T&&, storage_ptr)
template<class T>
value
to_value_impl_1(
    T&& t, storage_ptr sp,
    std::true_type)
{
    return value(
        std::forward<T>(t),
        std::move(sp));
}

template<class T>
value
to_value_impl_1(
    T&& t,
    storage_ptr sp,
    std::false_type)

{
    value jv(move(sp));
    to_value_impl_2(jv,
        std::forward<T>(t),
        has_to_value_traits<
            remove_cvref<T>>{});
    return jv;
}

} // detail
} // json
} // boost

#endif
