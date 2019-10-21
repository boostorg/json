//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_ASSERT_HPP
#define BOOST_JSON_DETAIL_ASSERT_HPP

#ifndef BOOST_JSON_STANDALONE
# include <boost/assert.hpp>
# define BOOST_JSON_ASSERT BOOST_ASSERT
#else
# include <cassert>
# define BOOST_JSON_ASSERT assert
#endif

#endif
