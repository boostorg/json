//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_SYSTEM_ERROR_HPP
#define BOOST_JSON_DETAIL_SYSTEM_ERROR_HPP

#ifndef BOOST_JSON_STANDALONE
# include <boost/system/error_code.hpp>
# include <boost/system/system_error.hpp>
#else
# include <system_error>
#endif

namespace boost {
namespace json {

#ifndef BOOST_JSON_STANDALONE
/// The type of error code used by the library
using error_code = boost::system::error_code;

/// The type of system error thrown by the library
using system_error = boost::system::system_error;

/// The type of error category used by the library
using error_category = boost::system::error_category;

/// The type of error condition used by the library
using error_condition = boost::system::error_condition;

#else
using error_code = std::error_code;
using system_error = std::system_error;
using error_category = std::error_category;
using error_condition = std::error_condition;

#endif

} // json
} // boost

#endif
