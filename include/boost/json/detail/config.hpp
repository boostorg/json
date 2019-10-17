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

#if defined(GENERATING_DOCUMENTATION)
# define BOOST_JSON_DECL
#elif defined(BOOST_JSON_HEADER_ONLY)
# define BOOST_JSON_DECL  inline
#else
# if (defined(BOOST_JSON_DYN_LINK) || defined(BOOST_ALL_DYN_LINK)) && !defined(BOOST_JSON_STATIC_LINK)
#  if defined(BOOST_JSON_SOURCE)
#   define BOOST_JSON_DECL  BOOST_SYMBOL_EXPORT
#   define BOOST_JSON_BUILD_DLL
#  else
#   define BOOST_JSON_DECL  BOOST_SYMBOL_IMPORT
#  endif
# endif // shared lib
# ifndef  BOOST_JSON_DECL
#  define BOOST_JSON_DECL
# endif
# if !defined(BOOST_JSON_SOURCE) && !defined(BOOST_ALL_NO_LIB) && !defined(BOOST_JSON_NO_LIB)
#  define BOOST_LIB_NAME boost_json
#  if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_JSON_DYN_LINK)
#   define BOOST_DYN_LINK
#  endif
#  include <boost/config/auto_link.hpp>
# endif  // auto-linking disabled
#endif

namespace boost {
namespace json {
namespace detail {

// max length of strings used as keys or values
constexpr unsigned long max_string_length_ = 0x7ffffffe; // 2GB

} // detail
} // json
} // boost

#endif
