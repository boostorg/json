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

#ifndef BOOST_JSON_VALUE_FROM_HPP
#define BOOST_JSON_VALUE_FROM_HPP

#include <boost/core/detail/static_assert.hpp>
#include <boost/json/detail/value_from.hpp>

namespace boost {
namespace json {

/** Convert an object of type `T` to @ref value.

    This function attempts to convert an object of type `T` to @ref value using

    - one of @ref value's constructors,
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
    void tag_invoke( value_from_tag, value&, T, const Context&, const FullContext& );

    void tag_invoke( value_from_tag, value&, T, const Context& );

    void tag_invoke( value_from_tag, value&, T );
    @endcode

    The `ctx` argument can be used either as a tag type to provide conversions
    for third-party types, or to pass extra data to the conversion function.

    Overloads **(2)** and **(4)** construct their return value using the
    @ref storage_ptr `sp`, which ensures that the memory resource is correctly
    propagated.

    @par Exception Safety
    Strong guarantee.

    @tparam T The type of the object to convert.

    @tparam Context The type of context passed to the conversion function.

    @param t The object to convert.

    @param ctx Context passed to the conversion function.

    @param jv @ref value out parameter.

    @see @ref value_from_tag, @ref value_to,
    <a href="http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1895r0.pdf">
        tag_invoke: A general pattern for supporting customisable functions</a>
*/
/// @{
template< class T, class Context >
void
value_from(
    T&& t,
    Context const& ctx,
    value& jv)
{
    using bare_T = detail::remove_cvref<T>;
    using cat = detail::value_from_category<Context, bare_T>;
    detail::value_from_impl( cat(), jv, std::forward<T>(t), ctx );
}

/** Overload
   @param t
   @param ctx
   @param sp A storage pointer referring to the memory resource to use for the
   returned @ref value.

   @return Overloads **(2)** and **(4)** return `t` converted to @ref value.
   Overloads **(1)** and **3** return `void` instead and pass their result via
   the out parameter `jv`.
*/
template< class T, class Context >
#ifndef BOOST_JSON_DOCS
typename std::enable_if<
    !std::is_same< detail::remove_cvref<Context>, storage_ptr >::value &&
    !std::is_same< detail::remove_cvref<Context>, value >::value,
    value >::type
#else
value
#endif
value_from(
    T&& t,
    Context const& ctx,
    storage_ptr sp = {})
{
    value jv(std::move(sp));
    value_from( static_cast<T&&>(t), ctx, jv );
    return jv;
}

/// Overload
template<class T>
void
value_from(
    T&& t,
    value& jv)
{
   value_from( static_cast<T&&>(t), detail::no_context(), jv );
}

/// Overload
template<class T>
value
value_from(
    T&& t,
    storage_ptr sp = {})
{
   return value_from(
           static_cast<T&&>(t), detail::no_context(), std::move(sp) );
}
/// @}

/** Determine if `T` can be converted to @ref value.

    If `T` can be converted to @ref value via a call to @ref value_from, the
    static data member `value` is defined as `true`. Otherwise, `value` is
    defined as `false`.

    @see @ref value_from.
*/
#ifdef BOOST_JSON_DOCS
template<class T>
using has_value_from = __see_below__;
#else
template<class T>
using has_value_from = detail::can_convert<
    detail::remove_cvref<T>, detail::value_from_conversion>;
#endif

} // namespace json
} // namespace boost

#endif
