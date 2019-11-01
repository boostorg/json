//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_CONFIG_HPP
#define BOOST_JSON_DETAIL_CONFIG_HPP

#include <boost/config.hpp>

#ifndef BOOST_JSON_STANDALONE
# if defined(GENERATING_DOCUMENTATION)
#  define BOOST_JSON_DECL
# elif defined(BOOST_JSON_HEADER_ONLY)
#  define BOOST_JSON_DECL  inline
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
#  endif  // auto-linking disabled
# endif
#else
# define BOOST_JSON_DECL  inline
# ifndef BOOST_JSON_HEADER_ONLY
#  error Standalone configuration requires BOOST_JSON_HEADER_ONLY
# endif
#endif

#ifndef BOOST_JSON_FORCEINLINE
# ifdef _MSC_VER
#  define BOOST_JSON_FORCEINLINE __forceinline
# else
#  define BOOST_JSON_FORCEINLINE
# endif
#endif

#define BOOST_JSON_VALUE_IS_TRIVIAL

#ifndef BOOST_NO_EXCEPTIONS
# define BOOST_JSON_THROW(x) throw(x)
#else
# define BOOST_JSON_THROW(x) do{}while(0)
#endif

#define BOOST_JSON_STATIC_ASSERT( ... ) static_assert(__VA_ARGS__, #__VA_ARGS__)

#endif
