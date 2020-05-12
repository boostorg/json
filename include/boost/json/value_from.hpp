//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_VALUE_FROM_HPP
#define BOOST_JSON_VALUE_FROM_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/value.hpp>
#include <boost/json/detail/value_from.hpp>

namespace boost {
namespace json {

/** Construct a JSON value from T.

    This function attempts to convert the instance of
    type `T` to a @ref value. It handles these cases,
    in order of decreasing priority:

    @li If @ref value is constructible `T` then the
        function returns `value( t, sp )`.

    @li If an overload of @ref tag_invoke is found
        by an unqualified call (i.e. by argument
        dependent lookup) invocable with this equivalent
        function signature:
        @code
        value tag_invoke( value_from_tag, T, storage_ptr )
        @endcode

    @li Otherwise if `T` matches any from a set of
        generic requirements implemented by the library,
        then the function returns a value generated
        from `t`.

    @par Constraints
    @code
    has_value_from< T >::value
    @endcode

    @par Exception Safety

    Strong guarantee.

    @param t The instance of `T` to convert.

    @param sp The storage to use for the returned value.

    @returns The JSON value representing `t`.

    @see @ref value_to
*/
template<class T>
value
value_from(
    T&& from,
    storage_ptr sp = {})
{
    return detail::value_from_impl(std::forward<T>(from),
        detail::move(sp), detail::priority_tag<16>());
}

/** Determine if T can be converted to a JSON value.

    This template type alias is `std::true_type` when one
    or more of the following are true:

    @li @ref value is constructible with the signature
        `(T, storage_ptr)`

    @li An overload of @ref tag_invoke is found
        by an unqualified call (i.e. by argument
        dependent lookup) invocable with this equivalent
        function signature:
        @code
        value tag_invoke( value_from_tag, T, storage_ptr )
        @endcode

    @li The type T matches one of the generic types
        supported by the library.

    Otherwise, this template type alias is `std::false_type`.
*/
#ifdef BOOST_JSON_DOCS
template<class T>
using has_value_from = __see_below__;
#else
template<class T, class>
struct has_value_from : std::false_type { };

template<class T>
struct has_value_from<T, detail::void_t<
    decltype(detail::value_from_impl(
        std::declval<T&&>(),
        std::declval<storage_ptr>(), 
        detail::priority_tag<16>()))>>
    : std::true_type { };
#endif

} // json
} // boost

#endif