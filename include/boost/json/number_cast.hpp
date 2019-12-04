//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_NUMBER_CAST_HPP
#define BOOST_JSON_NUMBER_CAST_HPP

#include <boost/json/config.hpp>
#include <boost/json/value.hpp>
#include <boost/json/detail/number_cast.hpp>
#include <type_traits>

namespace boost {
namespace json {

/** Returns a value converted to a numeric type.

    This function attempts to convert a @ref value to
    the numeric type `T` using these rules:

    @li If `! jv.is_number()`, an error is returned.

    @li If `T` is an integral type and the value can
    be represented exactly, it is returned. Otherwise
    the error is set.

    @li If `T` is a floating point type, the value
    returned is either exact or the closest approximation
    which is determined as-if using `static_cast<T>`.

    @par Constraints

    @code
    std::is_arithmetic<T>::value
    @endcode

    @tparam T The numeric type to return.

    @param jv The value to convert.

    @param ec Set to the error, if any occurred.
*/
template<class T>
#ifdef GENERATING_DOCUMENTATION
T
#else
typename std::enable_if<
    std::is_arithmetic<T>::value, T>::type
#endif
number_cast(value const& jv, error_code& ec)
{
    if(! jv.is_number())
    {
        ec = error::not_number;
        return {};
    }
    return detail::number_cast<T>(jv, ec);
}

/** Returns a value converted to a numeric type.

    This function attempts to convert a @ref value to
    the numeric type `T` using these rules:

    @li If `! jv.is_number()`, an exception is thrown.

    @li If `T` is an integral type and the value can
    be represented exactly, it is returned. Otherwise
    an exception is thrown.

    @li If `T` is a floating point type, the value
    returned is either exact or the closest approximation
    which is determined as-if using `static_cast<T>`.

    @par Constraints

    @code
    std::is_arithmetic<T>::value
    @endcode

    @tparam T The numeric type to return.

    @param jv The value to convert.

    @throws system_error on error.
*/
template<class T>
#ifdef GENERATING_DOCUMENTATION
T
#else
typename std::enable_if<
    std::is_arithmetic<T>::value, T>::type
#endif
number_cast(value const& jv)
{
    error_code ec;
    typename detail::remove_const<T>::type result;
    result = number_cast<T>(jv, ec);
    if(ec)
        BOOST_JSON_THROW(
            system_error(ec));
    return result;
}

} // json
} // boost

#endif
