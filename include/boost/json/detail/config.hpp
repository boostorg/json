//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_DETAIL_CONFIG_HPP
#define BOOST_JSON_DETAIL_CONFIG_HPP

#ifndef BOOST_JSON_STANDALONE
# include <boost/config.hpp>
# include <boost/assert.hpp>
# include <boost/throw_exception.hpp>
#else
# include <cassert>
#endif
#include <cstdint>

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
#ifndef BOOST_JSON_NODISCARD
# ifdef __has_cpp_attribute
// clang-6 accepts [[nodiscard]] with -std=c++14, but warns about it -pedantic
#  if __has_cpp_attribute(nodiscard) && !(defined(__clang__) && (__cplusplus < 201703L))
#   define BOOST_JSON_NODISCARD [[nodiscard]]
#  else
#   define BOOST_JSON_NODISCARD
#  endif
# else
#  define BOOST_JSON_NODISCARD
# endif
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

// BOOST_NORETURN ---------------------------------------------//
// Macro to use before a function declaration/definition to designate
// the function as not returning normally (i.e. with a return statement
// or by leaving the function scope, if the function return type is void).
#if !defined(BOOST_NORETURN)
#  if defined(_MSC_VER)
#    define BOOST_NORETURN __declspec(noreturn)
#  elif defined(__GNUC__)
#    define BOOST_NORETURN __attribute__ ((__noreturn__))
#  elif defined(__has_attribute) && defined(__SUNPRO_CC) && (__SUNPRO_CC > 0x5130)
#    if __has_attribute(noreturn)
#      define BOOST_NORETURN [[noreturn]]
#    endif
#  elif defined(__has_cpp_attribute) 
#    if __has_cpp_attribute(noreturn)
#      define BOOST_NORETURN [[noreturn]]
#    endif
#  endif
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

#ifndef BOOST_NOINLINE
# ifdef _MSC_VER
#  define BOOST_NOINLINE __declspec(noinline)
# elif defined(__GNUC__) || defined(__clang__)
#  define BOOST_NOINLINE __attribute__((noinline))
# else
#  define BOOST_NOINLINE
# endif
#endif

#ifndef BOOST_THROW_EXCEPTION
# ifndef BOOST_NO_EXCEPTIONS
#  define BOOST_THROW_EXCEPTION(x) throw(x)
# else
#  define BOOST_THROW_EXCEPTION(x) do{}while(0)
# endif
#endif

#if ! defined(BOOST_JSON_NO_SSE2) && \
    ! defined(BOOST_JSON_USE_SSE2)
# if (defined(_M_IX86) && _M_IX86_FP == 2) || \
      defined(_M_X64) || defined(__SSE2__)
#  define BOOST_JSON_USE_SSE2
# endif
#endif

#ifndef BOOST_SYMBOL_VISIBLE
#define BOOST_SYMBOL_VISIBLE
#endif

#ifdef BOOST_JSON_STANDALONE
# define BOOST_JSON_NS_BEGIN \
    namespace boost { \
    namespace json { \
    inline namespace standalone {
# define BOOST_JSON_NS_END } } }
#elif ! defined(BOOST_JSON_DOCS)
# define BOOST_JSON_NS_BEGIN \
    namespace boost { \
    namespace json {
# define BOOST_JSON_NS_END } }
#endif

#ifndef BOOST_JSON_STANDALONE
# if defined(BOOST_JSON_DOCS)
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
#   include <boost/config/auto_link.hpp>
#  endif
# endif
#else
# ifdef BOOST_JSON_HEADER_ONLY
#  define BOOST_JSON_DECL  inline
# else
#  define BOOST_JSON_DECL
# endif
#endif

#ifndef BOOST_JSON_LIKELY
# if defined(__GNUC__) || defined(__clang__)
#  define BOOST_JSON_LIKELY(x) __builtin_expect(!!(x), 1)
# else
#  define BOOST_JSON_LIKELY(x) x
# endif
#endif

#ifndef BOOST_JSON_UNLIKELY
# if defined(__GNUC__) || defined(__clang__)
#  define BOOST_JSON_UNLIKELY(x) __builtin_expect(!!(x), 0)
# else
#  define BOOST_JSON_UNLIKELY(x) x
# endif
#endif

// These macros are private, for tests, do not change
// them or else previously built libraries won't match.
#ifndef  BOOST_JSON_MAX_STRUCTURED_SIZE
# define BOOST_JSON_NO_MAX_STRUCTURED_SIZE
# define BOOST_JSON_MAX_STRUCTURED_SIZE  0x7ffffffe
#endif
#ifndef  BOOST_JSON_MAX_STRING_SIZE
# define BOOST_JSON_NO_MAX_STRING_SIZE
# define BOOST_JSON_MAX_STRING_SIZE  0x7ffffffe
#endif
#ifndef  BOOST_JSON_MAX_STACK_SIZE
# define BOOST_JSON_NO_MAX_STACK_SIZE
# define BOOST_JSON_MAX_STACK_SIZE  ((::size_t)(-1))
#endif
#ifndef  BOOST_JSON_PARSER_BUFFER_SIZE
# define BOOST_JSON_NO_PARSER_BUFFER_SIZE
# define BOOST_JSON_PARSER_BUFFER_SIZE 2048
#endif

BOOST_JSON_NS_BEGIN
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
BOOST_JSON_NS_END

#endif
