//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_SYSTEM_ERROR_HPP
#define BOOST_JSON_SYSTEM_ERROR_HPP

#include <boost/json/detail/config.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/result.hpp>
#include <boost/system/system_error.hpp>

BOOST_JSON_NS_BEGIN

/// The type of error code used by the library.
using error_code = boost::system::error_code;

/// The type of error category used by the library.
using error_category = boost::system::error_category;

/// The type of error condition used by the library.
using error_condition = boost::system::error_condition;

/// The type of system error thrown by the library.
using system_error = boost::system::system_error;

/** The type of result returned by library functions

    This is an alias template used as the return type for functions that can
    either return a value, or fail with an error code. This is a brief
    synopsis of the type:

    @par Declaration
    @code
    template< class T >
    class result
    {
    public:
        // Return true if the result contains an error
        constexpr bool has_error() const noexcept;

        // These two return true if the result contains a value
        constexpr bool has_value() const noexcept;
        constexpr explicit operator bool() const noexcept;

        // Return the value or throw an exception if has_value() == false
        constexpr T& value();
        constexpr T const& value() const;

        // Return the value, assuming the result contains it
        constexpr T& operator*();
        constexpr T const& operator*() const;

        // Return the error, which is default constructed if has_error() == false
        constexpr error_code error() const noexcept;
        ...more
    };
    @endcode

    @par Usage
    Given the function @ref try_value_to with this signature:

    @code
    template< class T>
    result< T > try_value_to( const value& jv );
    @endcode

    The following statement captures the value in a variable upon success,
    otherwise throws:
    @code
    int n = try_value_to<int>( jv ).value();
    @endcode

    This statement captures the result in a variable and inspects the error
    condition:
    @code
    result< int > r = try_value_to<int>( jv );
    if( r )
        std::cout << *r;
    else
        std::cout << r.error();
    @endcode

    @note For a full synopsis of the type, please see the corresponding
    documentation in Boost.System.

    @tparam T The type of value held by the result.
*/
template< class T >
using result = boost::system::result<T>;

#ifdef BOOST_JSON_DOCS
/// Returns the generic error category used by the library.
error_category const&
generic_category();
#else
using boost::system::generic_category;
#endif

BOOST_JSON_NS_END

#endif
