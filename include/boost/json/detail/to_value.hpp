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
        std::declval<value&>() =
            to_value_traits<T>::construct(
                std::declval<T&&>(),
                std::declval<storage_ptr const&>())
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
        std::declval<value&>() =
            std::declval<T const&>().to_json(
                std::declval<storage_ptr const&>())
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

template<class T>
typename std::enable_if<
    has_to_value<decltype(
        *std::declval<T const&>().begin())>::value &&
    std::is_same<
        decltype(std::declval<T const&>().begin()),
        decltype(std::declval<T const&>().end())>::value,
    value>::type
to_value_impl_4(
    T const& t, storage_ptr const& sp)
{
    array a(sp);
    for(auto const& e : t)
        a.emplace_back(to_value(e, sp));
    return a;
}

//----------------------------------------------------------

// Use T::to_value
template<class T>
auto
to_value_impl_3(
    T&& t, storage_ptr sp,
    std::true_type) -> decltype(
        std::forward<T>(
            t).to_json(move(sp)))
{
    return std::forward<T>(
        t).to_json(move(sp));
}

template<class T>
auto
to_value_impl_3(
    T&& t, storage_ptr sp,
    std::false_type) -> decltype(
        to_value_impl_4(
            std::forward<T>(t),
            std::move(sp)))
{
    return to_value_impl_4(
        std::forward<T>(t),
        std::move(sp));
}

//----------------------------------------------------------

// Use to_value_traits
template<class T>
auto
to_value_impl_2(
    T&& t, storage_ptr sp,
    std::true_type) -> decltype(
        to_value_traits<
            remove_cvref<T>>::construct(
                std::forward<T>(t),
                move(sp)))
{
    return to_value_traits<
        remove_cvref<T>>::construct(
            std::forward<T>(t),
            move(sp));
}

template<class T>
auto
to_value_impl_2(
    T&& t, storage_ptr sp,
    std::false_type) -> decltype(
        to_value_impl_3(
            std::forward<T>(t),
            detail::move(sp),
            has_to_json_mf<
                remove_cvref<T>>{}))
{
    return to_value_impl_3(
        std::forward<T>(t),
        detail::move(sp),
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
auto
to_value_impl_1(
    T&& t, storage_ptr sp,
    std::false_type) -> decltype(
        to_value_impl_2(
            std::forward<T>(t),
            detail::move(sp),
            has_to_value_traits<
                remove_cvref<T>>{}))
{
    return to_value_impl_2(
        std::forward<T>(t),
        detail::move(sp),
        has_to_value_traits<
            remove_cvref<T>>{});
}

} // detail
} // json
} // boost

#endif
