//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_TRAITS_HPP
#define BOOST_JSON_TRAITS_HPP

#include <boost/json/config.hpp>

namespace boost {
namespace json {

/** Customization point to construct a JSON value from a user-defined type.

    This class template is used by the implementation as
    needed to construct a @ref value from a user-defined
    type `T`. The customization point is used by declaring
    a specialization for the type `T` and providing a
    public static member function with this signature:

    @code
    void assign( value&, T const& t )
    @endcode

    The specialization must be in the same namespace as
    the library.

    @par Example

    This example declares a struct T and specializes the
    trait to provide a means of construction:

    @code

    struct T
    {
        int i;
        bool b;
    };

    // Specialization of to_value_traits must be in the
    // same namespace of the library in order to compile.

    namespace boost {
    namespace json {
    template<>
    struct to_value_traits< T >
    {
        static void assign( value& jv, T const& t )
        {
            jv = { t.i, t.b };
        }
    };
    } // namespace json
    } // naemspace boost

    @endcode

    @tparam T The type for which the conversion should
    be customized.

    @see @ref to_value, @ref value_cast, @ref value_cast_traits
*/
template<class T>
struct to_value_traits
{
};

//----------------------------------------------------------

/** Customization point to construct a user-defined type for a JSON value.
*/
template<class T>
struct value_cast_traits
{
};

} // json
} // boost

#endif
