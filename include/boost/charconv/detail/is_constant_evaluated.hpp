//  Copyright John Maddock 2011-2021.
//  Copyright Matt Borland 2023.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_CHARCONV_TOOLS_IS_CONSTANT_EVALUATED_HPP
#define BOOST_CHARCONV_TOOLS_IS_CONSTANT_EVALUATED_HPP

#include <boost/charconv/detail/config.hpp>
#include <type_traits>

#ifdef __cpp_lib_is_constant_evaluated
#  define BOOST_CHARCONV_HAS_IS_CONSTANT_EVALUATED
#endif

#ifdef __has_builtin
#  if __has_builtin(__builtin_is_constant_evaluated) && !defined(BOOST_NO_CXX14_CONSTEXPR)
#    define BOOST_CHARCONV_HAS_BUILTIN_IS_CONSTANT_EVALUATED
#  endif
#endif

//
// MSVC also supports __builtin_is_constant_evaluated if it's recent enough:
//
#if defined(_MSC_FULL_VER) && (_MSC_FULL_VER >= 192528326)
#  define BOOST_CHARCONV_HAS_BUILTIN_IS_CONSTANT_EVALUATED
#endif

//
// As does GCC-9:
//
#if !defined(BOOST_NO_CXX14_CONSTEXPR) && (__GNUC__ >= 9) && !defined(BOOST_CHARCONV_HAS_BUILTIN_IS_CONSTANT_EVALUATED)
#  define BOOST_CHARCONV_HAS_BUILTIN_IS_CONSTANT_EVALUATED
#endif

#if defined(BOOST_CHARCONV_HAS_IS_CONSTANT_EVALUATED) && !defined(BOOST_NO_CXX14_CONSTEXPR)
#  define BOOST_CHARCONV_IS_CONSTANT_EVALUATED(x) std::is_constant_evaluated()
#elif defined(BOOST_CHARCONV_HAS_BUILTIN_IS_CONSTANT_EVALUATED)
#  define BOOST_CHARCONV_IS_CONSTANT_EVALUATED(x) __builtin_is_constant_evaluated()
#elif !defined(BOOST_NO_CXX14_CONSTEXPR) && (__GNUC__ >= 6)
#  define BOOST_CHARCONV_IS_CONSTANT_EVALUATED(x) __builtin_constant_p(x)
#  define BOOST_CHARCONV_USING_BUILTIN_CONSTANT_P
#else
#  define BOOST_CHARCONV_IS_CONSTANT_EVALUATED(x) false
#  define BOOST_CHARCONV_NO_CONSTEXPR_DETECTION
#endif

#endif // BOOST_CHARCONV_TOOLS_IS_CONSTANT_EVALUATED_HPP
