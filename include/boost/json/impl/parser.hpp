//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_PARSER_HPP
#define BOOST_JSON_IMPL_PARSER_HPP

namespace boost {
namespace beast {
namespace json {

template<class T>
void
parser::
assign(T&& t)
{
    auto& jv = *stack_.front();
    BOOST_ASSERT(! jv.is_object());
    if(obj_)
    {
        BOOST_ASSERT(jv.is_null());
        jv = std::forward<T>(t);
        stack_.pop_front();
    }
    else if(stack_.front()->is_array())
    {
        BOOST_ASSERT(s_.empty());
        jv.as_array().emplace_back(
            std::forward<T>(t));
    }
    else
    {
        BOOST_ASSERT(jv.is_null());
        jv = std::forward<T>(t);
    }
}

} // json
} // beast
} // boost

#endif
