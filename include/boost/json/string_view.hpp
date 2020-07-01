//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_STRING_VIEW_HPP
#define BOOST_JSON_STRING_VIEW_HPP

#include <boost/json/detail/config.hpp>
#ifndef BOOST_JSON_STANDALONE
# include <boost/utility/string_view.hpp>
#else
# include <string_view>
#endif
#include <type_traits>

namespace boost {
namespace json {

#ifndef BOOST_JSON_STANDALONE

/// The type of string view used by the library.
using string_view = boost::string_view;

#else

using string_view = std::string_view;

#endif

namespace detail {

template<class T>
using is_string_viewish = typename std::enable_if<
    std::is_convertible<
        T const&, string_view>::value &&
    ! std::is_convertible<
        T const&, char const*>::value
            >::type;

} // detail

} // json
} // boost

#endif
