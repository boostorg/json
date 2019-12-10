//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_CONFIG_HPP
#define BOOST_JSON_DETAIL_CONFIG_HPP

#ifndef BOOST_JSON_STANDALONE
# include <boost/config.hpp>
# include <boost/assert.hpp>
# include <boost/system/error_code.hpp>
# include <boost/system/system_error.hpp>
# include <boost/utility/string_view.hpp>
# include <boost/throw_exception.hpp>
#else
# include <cassert>
# include <string_view>
# include <system_error>
#endif
#include <stdint.h>

// detect 32/64 bit
#if UINTPTR_MAX == UINT64_MAX
# define BOOST_JSON_ARCH 64
#elif UINTPTR_MAX == UINT32_MAX
# define BOOST_JSON_ARCH 32
#else
# error Unknown or unsupported architecture, please open an issue
#endif

// VFALCO Copied from Boost.Config
//        This is a derivative work.
#ifdef __has_cpp_attribute
// clang-6 accepts [[nodiscard]] with -std=c++14, but warns about it -pedantic
# if __has_cpp_attribute(nodiscard) && !(defined(__clang__) && (__cplusplus < 201703L))
#  define BOOST_JSON_NODISCARD [[nodiscard]]
# else
#  define BOOST_JSON_NODISCARD
# endif
#else
# define BOOST_JSON_NODISCARD
#endif

#ifndef BOOST_JSON_REQUIRE_CONST_INIT
# define BOOST_JSON_REQUIRE_CONST_INIT		
# if defined(__clang__) && defined(__has_cpp_attribute)		
#  if __has_cpp_attribute(clang::require_constant_initialization)		
#   undef BOOST_JSON_REQUIRE_CONST_INIT		
#   define BOOST_JSON_REQUIRE_CONST_INIT [[clang::require_constant_initialization]]		
#  endif
# endif
#endif

#ifndef BOOST_ASSERT
#define BOOST_ASSERT assert
#endif

#ifndef BOOST_STATIC_ASSERT
#define BOOST_STATIC_ASSERT( ... ) static_assert(__VA_ARGS__, #__VA_ARGS__)
#endif

#ifndef BOOST_FALLTHROUGH
#define BOOST_FALLTHROUGH [[fallthrough]]
#endif

#ifndef BOOST_FORCEINLINE
# ifdef _MSC_VER
#  define BOOST_FORCEINLINE __forceinline
# else
#  define BOOST_FORCEINLINE inline
# endif
#endif

#ifndef BOOST_THROW_EXCEPTION
# ifndef BOOST_NO_EXCEPTIONS
#  define BOOST_JSON_THROW(x) throw(x)
# else
#  define BOOST_JSON_THROW(x) do{}while(0)
# endif
#else
# define BOOST_JSON_THROW(x) BOOST_THROW_EXCEPTION(x)
#endif

#ifndef BOOST_JSON_NO_SSE2
# if (defined(_M_IX86) && _M_IX86_FP == 2) || \
      defined(_M_X64) || defined(__SSE2__)
#  define BOOST_JSON_USE_SSE2
# endif
#endif

#ifndef BOOST_JSON_STANDALONE
# if defined(GENERATING_DOCUMENTATION)
#  define BOOST_JSON_DECL
# elif defined(BOOST_JSON_HEADER_ONLY)
#  define BOOST_JSON_DECL inline
# else
#  if (defined(BOOST_JSON_DYN_LINK) || defined(BOOST_ALL_DYN_LINK)) && !defined(BOOST_JSON_STATIC_LINK)
#   if defined(BOOST_JSON_SOURCE)
#    define BOOST_JSON_DECL  BOOST_SYMBOL_EXPORT
#    define BOOST_JSON_BUILD_DLL
#   else
#    define BOOST_JSON_DECL  BOOST_SYMBOL_IMPORT
#   endif
#  endif // shared lib
#  ifndef  BOOST_JSON_DECL
#   define BOOST_JSON_DECL
#  endif
#  if !defined(BOOST_JSON_SOURCE) && !defined(BOOST_ALL_NO_LIB) && !defined(BOOST_JSON_NO_LIB)
#   define BOOST_LIB_NAME boost_json
#   if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_JSON_DYN_LINK)
#    define BOOST_DYN_LINK
#   endif
//#   include <boost/config/auto_link.hpp>
#  endif  // auto-linking disabled
# endif
#else
# ifdef BOOST_JSON_HEADER_ONLY
#  define BOOST_JSON_DECL  inline
# else
#  define BOOST_JSON_DECL
# endif
#endif

// These macros are private, for tests, do not change
// them or else previously built libraries won't match.
#ifndef BOOST_JSON_MAX_OBJECT_SIZE
# define BOOST_JSON_NO_MAX_OBJECT_SIZE
# define BOOST_JSON_MAX_OBJECT_SIZE  0x7ffffffe
#endif
#ifndef BOOST_JSON_MAX_ARRAY_SIZE
# define BOOST_JSON_NO_MAX_ARRAY_SIZE
# define BOOST_JSON_MAX_ARRAY_SIZE   0x7ffffffe
#endif
#ifndef BOOST_JSON_MAX_STRING_SIZE
# define BOOST_JSON_NO_MAX_STRING_SIZE
# define BOOST_JSON_MAX_STRING_SIZE  0x7ffffffe
#endif
#ifndef BOOST_JSON_MAX_STACK_SIZE
# define BOOST_JSON_NO_MAX_STACK_SIZE
# define BOOST_JSON_MAX_STACK_SIZE  ((::size_t)(-1))
#endif
#ifndef BOOST_JSON_PARSER_BUFFER_SIZE
# define BOOST_JSON_NO_PARSER_BUFFER_SIZE
# define BOOST_JSON_PARSER_BUFFER_SIZE 2048
#endif

namespace boost {
namespace json {

#ifndef BOOST_JSON_STANDALONE
using string_view = boost::string_view;
#else
using string_view = std::string_view;
#endif

namespace detail {

template<class...>
struct make_void
{
    using type =void;
};

template<class... Ts>
using void_t = typename
    make_void<Ts...>::type;

template<class T>
struct remove_const
{
    using type = T;
};

template<class T>
struct remove_const<T const>
{
    using type = T;
};

template<class T>
struct remove_volatile
{
    using type = T;
};

template<class T>
struct remove_volatile<T volatile>
{
    using type = T;
};

template<class T>
struct remove_reference
{
    using type = T;
};

template<class T>
struct remove_reference<T&>
{
    using type = T;
};

template<class T>
using remove_cv = typename
    remove_const<typename
    remove_reference<T>::type>::type;

template<class T>
using remove_cvref =
    typename remove_reference<
        remove_cv<T>>::type;

template<class T>
constexpr
typename remove_reference<T>::type&&
move(T&& t) noexcept
{
    return static_cast<typename
        remove_reference<T>::type&&>(t);
}

template<class T, class U>
inline
T
exchange(T& t, U u) noexcept
{
    T v = move(t);
    t = move(u);
    return v;
}

template<class T>
using is_string_viewish = typename std::enable_if<
    std::is_convertible<
        T const&, string_view>::value &&
    ! std::is_convertible<
        T const&, char const*>::value
            >::type;

/*  This is a derivative work, original copyright:

    Copyright Eric Niebler 2013-present

    Use, modification and distribution is subject to the
    Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)

    Project home: https://github.com/ericniebler/range-v3
*/
template<typename T>
struct static_const
{
    static constexpr T value {};
};
template<typename T>
constexpr T static_const<T>::value;

#define BOOST_JSON_INLINE_VARIABLE(name, type) \
    namespace { constexpr auto& name = \
        ::boost::json::detail::static_const<type>::value; \
    } struct _unused_ ## name ## _semicolon_bait_

} // detail
} // json
} // boost

#endif
