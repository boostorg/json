//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_VALUE_TO_HPP
#define BOOST_JSON_VALUE_TO_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/value.hpp>
#include <boost/json/detail/value_to.hpp>
#include <type_traits>

namespace boost {
namespace json {

//template<class> struct value_to_tag {};

/** Convert a JSON value to another type T.

    This function attempts to convert the @ref value
    to the given type `T`. It handles these cases, in
    order of decreasing priority:

    @li If @ref value_to_traits contains a specialization
        for `T`, then the function returns `T::from_json(jv)`.

    @li If `T` is numeric, the function returns `number_cast<T>(jv)`.

    @li If `T` is any of the other types that are directly
        represented in a @ref value (@ref object, @ref array,
        @ref string, or `bool`), and the kind of value is
        suitable then a `T const&` is returned. If the kind
        is not suitable then an exception is thrown.

    @li Otherwise, if `T` matches a generic container known
      to the implementation it is converted.

    @par Constraints
    @code
    has_value_to< T >::value
    @endcode

    @par Exception Safety

    Strong guarantee.

    @param jv The JSON value to convert.

    @tparam T The type to convert to.

    @returns The result of converting `jv` to a `T`.

    @see @ref value_from
*/
template<class T
#ifndef BOOST_JSON_DOCS
    , typename std::enable_if<
        ! std::is_reference<T>::value>::type* = nullptr
#endif
>
auto
value_to(value const& jv)
#ifndef BOOST_JSON_DOCS
    -> decltype(detail::value_to_impl(jv,
        value_to_tag< detail::remove_cv<T>>(), 
            detail::priority_tag<16>()))
#endif
{
    return detail::value_to_impl(jv,
        value_to_tag<detail::remove_cv<T>>(), 
            detail::priority_tag<16>());
}

/** Determine if T can be constructed from a JSON value.
*/
#ifdef BOOST_JSON_DOCS
template<class T>
using has_value_to = __see_below__;
#else
template<class T, class>
struct has_value_to
    : std::false_type { };

template<class T>
struct has_value_to<T, detail::void_t<
    decltype(value_to<T>(
        std::declval<const value&>()))>>
    : std::true_type { };
#endif

} // json
} // boost

#endif
