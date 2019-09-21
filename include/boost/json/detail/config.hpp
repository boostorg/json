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

//#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
//#include <boost/core/ignore_unused.hpp>

// Default to a header-only implementation. The user must specifically
// request separate compilation by defining BOOST_JSON_SEPARATE_COMPILATION
#ifndef BOOST_JSON_HEADER_ONLY
# ifndef BOOST_JSON_SEPARATE_COMPILATION
#   define BOOST_JSON_HEADER_ONLY 1
# endif
#endif

#if BOOST_BEAST_DOXYGEN
# define BOOST_JSON_DECL
#elif defined(BOOST_JSON_HEADER_ONLY)
# define BOOST_JSON_DECL inline
#else
# define BOOST_JSON_DECL
#endif

#endif
