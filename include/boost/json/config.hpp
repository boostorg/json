//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_CONFIG_HPP
#define BOOST_JSON_CONFIG_HPP

#include <boost/json/detail/config.hpp>

namespace boost {
namespace json {

#ifndef BOOST_JSON_STANDALONE

/// The type of string view used by the library.
using string_view = boost::string_view;

/// The type of error code used by the library.
using error_code = boost::system::error_code;

/// The type of system error thrown by the library.
using system_error = boost::system::system_error;

/// The type of error category used by the library.
using error_category = boost::system::error_category;

/// The type of error condition used by the library.
using error_condition = boost::system::error_condition;

#ifdef BOOST_JSON_DOCS
/// Returns the generic error category used by the library.
error_category const&
generic_category();
#else
using boost::system::generic_category;
#endif

#else

using error_code = std::error_code;
using error_category = std::error_category;
using error_condition = std::error_condition;
using string_view = std::string_view;
using system_error = std::system_error;
using std::generic_category;

#endif

} // json
} // boost

#endif
