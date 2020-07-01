//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_PILFER_HPP
#define BOOST_PILFER_HPP

#include <type_traits>
#include <utility>

/*
    Implements "pilfering" from P0308R0

    "Valueless Variants Considered Harmful"
    http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0308r0.html
    Author: Peter Dimov
*/

namespace boost {

template<class T>
class pilfered
{
    T& t_;

public:
    explicit
    constexpr
    pilfered(T&& t) noexcept
        : t_(t)
    {
    }

    constexpr T&
    get() const noexcept
    {
        return t_;
    }

    constexpr T*
    operator->() const noexcept
    {
        //return std::addressof(t_);
        return reinterpret_cast<T*>(
            const_cast<char *>(
                &reinterpret_cast<
                    const volatile char &>(t_)));
    }
};

namespace detail {

template<class>
struct not_pilfered
{
};

} // detail

template<class T>
struct is_pilfer_constructible :
    std::integral_constant<bool,
        std::is_nothrow_move_constructible<T>::value ||
        (
            std::is_nothrow_constructible<
                T, pilfered<T> >::value &&
            ! std::is_nothrow_constructible<
                T, detail::not_pilfered<T> >::value
        )>
{
};

template<class T>
auto
pilfer(T&& t) noexcept ->
    typename std::conditional<
        std::is_nothrow_constructible<
            typename std::remove_reference<T>::type,
            pilfered<typename std::remove_reference<T>::type> >::value &&
        ! std::is_nothrow_constructible<
            typename std::remove_reference<T>::type,
            detail::not_pilfered<typename std::remove_reference<T>::type> >::value,
        pilfered<typename std::remove_reference<T>::type>,
        typename std::remove_reference<T>::type&&
            >::type
{
    using U =
        typename std::remove_reference<T>::type;
    static_assert(
        is_pilfer_constructible<U>::value, "");
    return typename std::conditional<
        std::is_nothrow_constructible<
            U, pilfered<U> >::value &&
        ! std::is_nothrow_constructible<
            U, detail::not_pilfered<U> >::value,
        pilfered<U>, U&&
            >::type(std::move(t));
}

template<class T>
void
relocate(T* dest, T& src) noexcept
{
    static_assert(
        is_pilfer_constructible<T>::value, "");
    ::new(dest) T(pilfer(src));
    src.~T();
}

} // boost

#endif
