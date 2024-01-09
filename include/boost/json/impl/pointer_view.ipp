//
// Copyright (c) 2022 Dmitry Arkhipov (grisumbras@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_POINTER_VIEW_IPP
#define BOOST_JSON_IMPL_POINTER_VIEW_IPP

#include <boost/json/impl/pointer.ipp>
#include <boost/json/value_view.hpp>

namespace boost {
namespace json {



boost::optional<value_view>
value_view::find_pointer( string_view sv, error_code& ec ) const noexcept
{
    BOOST_ASSERT(!"not implemented");
    return boost::none;
}

boost::optional<value_view>
value_view::find_pointer(string_view ptr, std::error_code& ec) const noexcept
{
    error_code jec;
    auto res =  find_pointer(ptr, jec);
    ec = jec;
    return res;
}


} // namespace json
} // namespace boost

#endif // BOOST_JSON_IMPL_POINTER_IPP
