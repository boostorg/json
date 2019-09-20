    //
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_BASIC_PARSER_HPP
#define BOOST_JSON_IMPL_BASIC_PARSER_HPP

#include <boost/beast/core/buffers_range.hpp>

namespace boost {
namespace beast {
namespace json {

template<class ConstBufferSequence, class>
std::size_t
basic_parser::
write_some(
    ConstBufferSequence const& buffers,
    error_code& ec)
{
    static_assert(
        net::is_const_buffer_sequence<ConstBufferSequence>::value,
        "ConstBufferSequence type requirements not met");

    std::size_t bytes_used = 0;
    for(auto const b : beast::buffers_range_ref(buffers))
    {
        bytes_used += write_some(b, ec);
        if(ec)
            break;
    }
    return bytes_used;
}

template<class ConstBufferSequence, class>
std::size_t
basic_parser::
write(
    ConstBufferSequence const& buffers,
    error_code& ec)
{
    static_assert(
        net::is_const_buffer_sequence<ConstBufferSequence>::value,
        "ConstBufferSequence type requirements not met");

    std::size_t bytes_used = 0;
    auto it =
        net::buffer_sequence_begin(buffers);
    auto end =
        net::buffer_sequence_end(buffers);
    if(it == end)
    {
        ec = {};
        return 0;
    }
    for(--end; it != end; ++it)
    {
        bytes_used +=
            write_some(*it, ec);
        if(ec)
            return bytes_used;
    }
    bytes_used += write(*it, ec);
    return bytes_used;
}

} // json
} // beast
} // boost

#endif
