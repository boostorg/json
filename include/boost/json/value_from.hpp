//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_VALUE_FROM_HPP
#define BOOST_JSON_VALUE_FROM_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/value.hpp>
#include <boost/json/detail/value_from.hpp>

namespace boost {
namespace json {

/** Customization point tag.
    
    The tag-type used to identify `tag_invoke` overloads
    called by @ref value_from.

    @note This type is empty; it has no members.
*/
struct value_from_tag
#ifdef BOOST_JSON_DOCS
    { }
#endif
;

/** Convert an object of type `T` to @ref value.

    This function attempts to convert an object
    of type `T` to @ref value using

    @li one of @ref value's constructors,

    @li a library-provided generic conversion, or

    @li a user-provided overload of `tag_invoke`.
    
    In all cases, the conversion is done by calling
    an overload of `tag_invoke`, passing an lvalue `jv`
    of type @ref value as an argument to ensure that
    the result of the conversion uses the correct
    `storage_ptr`.@n

    The function used to convert an expression `e`
    of type `T` is determined as follows.
    Let _S_ be the set of all declarations found
    by argument-dependent lookup for the function call

    @code
    tag_invoke( value_from_tag(), jv, e )
    @endcode

    Then, an attempt to find an applicable library-provided
    conversion (if any) is made. Add to _S_

    @li an additional function designated the
    _value assignment candidate_ if
    `std::is_assignable<value&, T&&>::value`, otherwise,

    @li an additional function designated the
    _tuple conversion candidate_ if `T`
    satisfies _TupleLike_, otherwise,

    @li an additional function designated the
    _map conversion candidate_ if `T`
    satisfies _FromMapLike_, otherwise,

    @li an additional function designated the
    _container conversion candidate_ if `T`
    satisfies _FromContainerLike_, otherwise,

    If a library-provided conversion is added to
    _S_, it has the form

    @code
    template<typename U>
    void tag_invoke( value_from_tag, value&, U&& );
    @endcode

    Using _S_, overload resolution is performed to
    find the single best candidate to convert `e`
    to @ref value. If more than one function is found
    or if no viable function is found, the conversion
    fails. Otherwise, the selected function `F` is called
    with `F( value_from_tag(), jv, std::forward< T >( from ) )`
    and the value of `jv` after modification by `F` is
    returned as the result of the conversion.

    @par Exception Safety

    Strong guarantee.

    @tparam T The type of the object to convert.
    
    @returns `t` converted to @ref value.

    @param t The object to convert.

    @param sp A storage pointer referring to the memory resource
    to use for the returned @ref value. The default argument for this
    parameter is `{}`.

    @see @ref value_to, http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1895r0.pdf
*/
template<class T>
value
value_from(
    T&& t,
    storage_ptr sp = {})
{
    return detail::value_from_impl(std::forward<T>(t),
        detail::move(sp));
}

/** Determine if `T` can be converted to @ref value.

    Given an expression `e` of type `T`, 
    if overload resolution for the set of `tag_invoke`
    overloads performed by the function call
    
    @code 
    value_from( e )
    @endcode
    
    would find a single best `tag_invoke` function to call, 
    then this class template inherits from `std::true_type`;
    otherwise, it inherits from `std::false_type`.
*/
#ifdef BOOST_JSON_DOCS
template<class T>
using has_value_from = __see_below__;
#else
template<class T, class>
struct has_value_from : std::false_type { };

template<class T>
struct has_value_from<T, detail::void_t<
    decltype(detail::value_from_impl(std::declval<T&&>(),
        std::declval<storage_ptr>()))>>
    : std::true_type { };
#endif

} // json
} // boost

#endif