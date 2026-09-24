//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
// Copyright (c) 2022 Dmitry Arkhipov (grisumbras@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_VALUE_TO_HPP
#define BOOST_JSON_VALUE_TO_HPP

#include <boost/core/detail/static_assert.hpp>
#include <boost/json/detail/value_to.hpp>

namespace boost {
namespace json {

/** Convert a @ref value to an object of type `T`.

    This function attempts to convert a @ref value to `T` using

    - one of @ref value's accessors, or
    - a library-provided generic conversion, or
    - a user-provided overload of `tag_invoke`.

    In order to perform the conversion the function selects an appropriate
    implementation based on the types `T` and `Context` (if provided).

    1. If `Context` is available and is not `std::tuple<C...>`

       a. check if `use_category<T, Context>::value` is not
          @ref conversion_category::unknown; otherwise

       b. check if a `tag_invoke` overload from the list below that takes a
          `Context const&` exists.

    2. Otherwise, if `Context` is available, and is `std::tuple<C...>` repeat
       steps **1** and **2** recursively for every `C` until either
       step **1.a** or **1.b** succeeds for some `C`.

    3. Failing that,

       a. check if `use_category<T>::value` is not
          @ref conversion_category::unknown; otherwise

       b. check if a `tag_invoke` overload from the list below that takes only
          2 parameters exists; otherwise

       c. check if `T` is one of @ref value, @ref array, @ref object,
          or @ref string; otherwise

       d. check if `T` matches one of the categories of types described in the
          table "Conversion categories" in \<\<Value Conversion>> section.

    These steps determine both the appropriate category of conversion for `T`,
    and, if necessary, the effective context `C` that will be used for
    conversion. If the category is selected on steps **1.a**, **3.a**, **3.c**,
    or **3.d**, the library provides a suitable conversion implementation.
    If the category is selected on steps **2.b** or **3.b**, then a
    user-provided `tag_invoke` overload is used.

    The overloads of `tag_invoke` that will be considered by this function
    are in the following list. Overloads that appear higher in the list have
    higher priority.

    @code
    template< class FullContext >
    result<T> tag_invoke( try_value_to_tag<T>, const value&, const Context&, const FullContext& );

    template< class FullContext >
    T tag_invoke( value_to_tag<T>, const value&, const Context&, const FullContext& );

    result<T> tag_invoke( try_value_to_tag<T>, const value&, const Context& );

    T tag_invoke( value_to_tag<T>, const value&, const Context& );

    result<T> tag_invoke( try_value_to_tag<T>, const value& );

    T tag_invoke( value_to_tag<T>, const value& );
    @endcode

    For `tag_invoke` overloads that take a parameter of type `value_to_tag<T>`
    the object returned by a call to that overload is returned by the function
    as the result of the conversion. For overloads that take a parameter of
    type `try_value_to_tag<T>` if the returned `result` contains a value, that
    value is returned as the result of the conversion. Otherwise, an exception
    of type @ref boost::system::system_error that stores the error is thrown.

    The `ctx` argument can be used either as a tag type to provide conversions
    for third-party types, or to pass extra data to the conversion function.

    Overload **(3)** is **deleted** and participates in overload resolution
    only when `U` is not @ref value. The overload exists to prevent unintented
    creation of temporary @ref value instances, e.g.

    @code
    auto flag = value_to<bool>(true);
    @endcode

    @par Constraints
    @code
    ! std::is_reference< T >::value
    @endcode

    @par Exception Safety
    Strong guarantee.

    @tparam T The type to convert to.

    @tparam Context The type of context passed to the conversion function.

    @returns `jv` converted to `T`.

    @param jv The @ref value to convert.

    @param ctx Context passed to the conversion function.

    @see @ref try_value_to, @ref value_from,
    <a href="http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1895r0.pdf">
        tag_invoke: A general pattern for supporting customisable functions</a>

    @{
*/
template< class T, class Context >
T
value_to( value const& jv, Context const& ctx )
{
    BOOST_CORE_STATIC_ASSERT( ! std::is_reference<T>::value );
    using bare_T = detail::remove_cvref<T>;
    using cat = detail::value_to_category<Context, bare_T>;
    return detail::value_to_impl( cat(), value_to_tag<bare_T>(), jv, ctx );
}

/// Overload
template<class T>
T
value_to(const value& jv)
{
    return value_to<T>( jv, detail::no_context() );
}

/// Overload
template<class T, class U
#ifndef BOOST_JSON_DOCS
    , class = typename std::enable_if<!std::is_same<U, value>::value>::type
#endif
>
T
value_to(U const& jv) = delete;
/// @}

/** Convert a @ref value to a @ref boost::system::result.

    This function attempts to convert a @ref value to `result<T>` using

    - one of @ref value's accessors, or
    - a library-provided generic conversion, or
    - a user-provided overload of `tag_invoke`.

    In order to perform the conversion the function selects an appropriate
    implementation based on the types `T` and `Context` (if provided).

    1. If `Context` is available and is not `std::tuple<C...>`

       a. check if `use_category<T, Context>::value` is not
          @ref conversion_category::unknown; otherwise

       b. check if a `tag_invoke` overload from the list below that takes a
          `Context const&` exists.

    2. Otherwise, if `Context` is available, and is `std::tuple<C...>` repeat
       steps **1** and **2** recursively for every `C` until either
       step **1.a** or **1.b** succeeds for some `C`.

    3. Failing that,

       a. check if `use_category<T>::value` is not
          @ref conversion_category::unknown; otherwise

       b. check if a `tag_invoke` overload from the list below that takes only
          2 parameters exists; otherwise

       c. check if `T` is one of @ref value, @ref array, @ref object,
          or @ref string; otherwise

       d. check if `T` matches one of the categories of types described in the
          table "Conversion categories" in \<\<Value Conversion>> section.

    These steps determine both the appropriate category of conversion for `T`,
    and, if necessary, the effective context `C` that will be used for
    conversion. If the category is selected on steps **1.a**, **3.a**, **3.c**,
    or **3.d**, the library provides a suitable conversion implementation.
    If the category is selected on steps **2.b** or **3.b**, then a
    user-provided `tag_invoke` overload is used.

    The overloads of `tag_invoke` that will be considered by this function
    are in the following list. Overloads that appear higher in the list have
    higher priority.

    @code
    template< class FullContext >
    result<T> tag_invoke( try_value_to_tag<T>, const value&, const Context&, const FullContext& );

    template< class FullContext >
    T tag_invoke( value_to_tag<T>, const value&, const Context&, const FullContext& );

    result<T> tag_invoke( try_value_to_tag<T>, const value&, const Context& );

    T tag_invoke( value_to_tag<T>, const value&, const Context& );

    result<T> tag_invoke( try_value_to_tag<T>, const value& );

    T tag_invoke( value_to_tag<T>, const value& );
    @endcode

    For `tag_invoke` overloads that take a parameter of type
    `try_value_to_tag<T>` the object returned by a call to that overload is
    returned by the function as the result of the conversion. For overloads
    that take a parameter of type `value_to_tag<T>` the returned object is
    wrapped with a `result<T>`.

    If an error occurs during conversion, the result will store the error code
    associated with the error. If an exception is thrown, the function will
    attempt to retrieve the associated error code and return it, otherwise it
    will return @ref error::exception, unless the exception type is
    @ref std::bad_alloc, which will be allowed to propagate.

    The `ctx` argument can be used either as a tag type to provide conversions
    for third-party types, or to pass extra data to the conversion function.

    @par Constraints
    @code
    ! std::is_reference< T >::value
    @endcode

    @par Exception Safety
    Strong guarantee.

    @tparam T The type to convert to.
    @tparam Context The type of context passed to the conversion function.

    @param jv The @ref value to convert.
    @param ctx Context passed to the conversion function.

    @returns `jv` converted to `result<T>`.

    @see @ref value_to_tag, @ref value_to, @ref value_from,
         [tag_invoke: A general pattern for supporting customisable functions](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1895r0.pdf).

    @{
*/
template< class T, class Context >
typename result_for<T, value>::type
try_value_to( value const& jv, Context const& ctx )
{
    BOOST_CORE_STATIC_ASSERT( ! std::is_reference<T>::value );
    using bare_T = detail::remove_cvref<T>;
    using cat = detail::value_to_category<Context, bare_T>;
    return detail::value_to_impl(
        cat(), try_value_to_tag<bare_T>(), jv, ctx );
}

/// Overload
template<class T>
typename result_for<T, value>::type
try_value_to(const value& jv)
{
    return try_value_to<T>( jv, detail::no_context() );
}
/// @}

/** Determine if a @ref value can be converted to `T`.

    If @ref value can be converted to `T` via a
    call to @ref value_to, the static data member `value`
    is defined as `true`. Otherwise, `value` is
    defined as `false`.

    @see @ref value_to
*/
#ifdef BOOST_JSON_DOCS
template<class T>
using has_value_to = __see_below__;
#else
template<class T>
using has_value_to = detail::can_convert<
    detail::remove_cvref<T>, detail::value_to_conversion>;
#endif

} // namespace json
} // namespace boost

#endif
