//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_IMPL_SERIALIZE_IPP
#define BOOST_JSON_DETAIL_IMPL_SERIALIZE_IPP

#include <boost/json/detail/serialize.hpp>

namespace boost {
namespace json {
namespace detail {

void
serialize_impl(
    std::string& s,
    serializer& sr)
{
    // serialize to a small buffer to avoid
    // the first few allocations in std::string
    char buf[BOOST_JSON_STACK_BUFFER_SIZE];
    string_view sv;
    sv = sr.read(buf);
    if(sr.done())
    {
        // fast path
        s.append(
            sv.data(), sv.size());
        return;
    }
    std::size_t len = sv.size();
    s.reserve(len * 2);
    s.resize(s.capacity());
    BOOST_ASSERT(
        s.size() >= len * 2);
    std::memcpy(&s[0],
        sv.data(), sv.size());
    auto const lim =
        s.max_size() / 2;
    for(;;)
    {
        sv = sr.read(
            &s[0] + len,
            s.size() - len);
        len += sv.size();
        if(sr.done())
            break;
        // growth factor 2x
        if(s.size() < lim)
            s.resize(s.size() * 2);
        else
            s.resize(2 * lim);
    }
    s.resize(len);
}

} // detail
} // json
} // boost

#endif
