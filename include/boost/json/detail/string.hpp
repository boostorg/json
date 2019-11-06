//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_STRING_HPP
#define BOOST_JSON_DETAIL_STRING_HPP

#ifndef BOOST_JSON_STANDALONE
# include <boost/utility/string_view.hpp>
#else
# include <string_view>
#endif

namespace boost {
namespace json {

#ifndef BOOST_JSON_STANDALONE
/// The type of string view used by the library
using string_view = boost::string_view;

#else
using string_view = std::string_view;

#endif

namespace detail {

// max length of strings used as keys or values
constexpr size_type max_string_length_ = 0x7ffffffe; // 2GB

template<class T>
using is_viewy = typename std::enable_if<
    std::is_convertible<
        T const&, string_view>::value &&
    ! std::is_convertible<
        T const&, char const&>::value
            >::type;

} // detail

} // json
} // boost

#endif
