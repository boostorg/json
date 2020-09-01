//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_VALUE_TO_HPP
#define BOOST_JSON_VALUE_TO_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/value.hpp>
#include <boost/json/detail/value_to.hpp>

BOOST_JSON_NS_BEGIN

/** Customization point tag type.

    This tag type is used by the function
    @ref value_to to select overloads
    of `tag_invoke`.

    @note This type is empty; it has no members.

    @see
        @ref value_from
        @ref value_from_tag
        @ref value_to
*/
template<class T>
struct value_to_tag;

/** Convert a @ref value to an object of type `T`.

    This function attempts to convert a @ref value
    to `T` using

    @li one of `T`'s constructors,

    @li a library-provided generic conversion, or

    @li a user-provided overload of `tag_invoke`.
    
    In all cases, the conversion is done by calling
    an overload of `tag_invoke`.

    The function used to convert an expression `e`
    of type @ref value is determined as follows.
    Let _S_ be the set of all declarations found
    by argument-dependent lookup for the function call

    @code
    tag_invoke( value_to_tag<T>(), jv )
    @endcode
    
    Then, an attempt to find an applicable library-provided
    conversion (if any) is made. Add to _S_

    @li an additional function designated the
    _built-in candidate_ if `T` is @ref value, @ref object,
    @ref array, @ref string or `bool`, or if
    `std::is_arithmetic<T>::value` is `true`, otherwise,

    @li an additional function designated the
    _constructor candidate_ if
    `std::is_constructible<T, const value&>::value`, otherwise,

    @li an additional function designated the
    _string conversion candidate_ if `T`
    satisfies _StringLike_, otherwise,

    @li an additional function designated the
    _map conversion candidate_ if `T`
    satisfies _ToMapLike_, otherwise,

    @li an additional function designated the
    _container conversion candidate_ if `T`
    satisfies _ToContainerLike_, otherwise,

    If a library-provided conversion is added to
    _S_, it has the form

    @code
    template<typename U>
    U tag_invoke( value_to_tag<T>, const value& );
    @endcode

    Using _S_, overload resolution is performed to
    find the single best candidate to convert `jv`
    to `T`. If more than one function is found
    or if no viable function is found, the conversion
    fails. Otherwise, the selected function `F` is called
    with `F( value_to_tag<T>(), jv )` and its result is
    returned as the result of the conversion.

    @par Constraints
    @code
    ! std::is_reference< T >::value
    @endcode

    @par Exception Safety

    Strong guarantee.

    @tparam T The type to convert to.
    
    @returns `jv` converted to `T`.

    @param jv The @ref value to convert.

    @see @ref value_from, http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1895r0.pdf
*/
template<class T
#ifndef BOOST_JSON_DOCS
    , typename std::enable_if<
        !std::is_reference<T>::value>::type*
#endif
>
T
value_to(value const& jv)
{
    return detail::value_to_impl(
        value_to_tag<detail::remove_cv<T>>(), jv);
}

/** Determine if `T` can be converted to a JSON value.

    Given an expression `jv` of type @ref value,
    if overload resolution for the set of `tag_invoke`
    overloads performed by the function call

    @code
    value_to< T >( jv )
    @endcode

    would find a single best `tag_invoke` function to call, 
    then this class template inherits from `std::true_type`;
    otherwise, it inherits from `std::false_type`.

    @see @ref value_to
*/
#ifdef BOOST_JSON_DOCS
template<class T>
using has_value_to = __see_below__;
#else
template<class T, class>
struct has_value_to
    : std::false_type { };

template<class T>
struct has_value_to<T, detail::void_t<decltype(detail::value_to_impl(
    value_to_tag<detail::remove_cv<T>>(), std::declval<const value&>())),
        typename std::enable_if<!std::is_reference<T>::value>::type>>
    : std::true_type { };
#endif

BOOST_JSON_NS_END

#endif
