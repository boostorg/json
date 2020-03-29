//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_VALUE_CAST_HPP
#define BOOST_JSON_VALUE_CAST_HPP

#include <boost/json/config.hpp>
#include <boost/json/traits.hpp>
#include <boost/json/value.hpp>
#include <boost/json/detail/value_cast.hpp>
#include <type_traits>

namespace boost {
namespace json {

/** Convert a JSON value to another type T.

    This function attempts to convert the @ref value
    to the given type `T`. It handles these cases, in
    order of decreasing priority:

    @li If @ref value_cast_traits contains a specialization
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
    has_value_cast< T >::value
    @endcode

    @par Exception Safety

    Strong guarantee.

    @param jv The JSON value to convert.

    @tparam T The type to convert to.

    @returns The result of converting `jv` to a `T`.

    @see @ref to_value
*/
template<class T>
auto
value_cast(value const& jv)
#ifndef BOOST_JSON_DOCS
    -> decltype(
        detail::value_cast_impl_1<T>(jv,
        detail::has_value_cast_traits<T>{}))
#endif
{
    return
        detail::value_cast_impl_1<T>(jv,
        detail::has_value_cast_traits<T>{});
}

/** Determine if T can be constructed from a JSON value.
*/
#ifdef BOOST_JSON_DOCS
template<class T>
using has_value_cast = __see_below__;
#else

template<class T, class = void>
struct has_value_cast : std::false_type
{
};

template<class T>
struct has_value_cast<T,
    detail::void_t<decltype(
        value_cast<T>(
            std::declval<value const&>())
    ) > > : std::true_type
{
};

#endif

} // json
} // boost

#endif
