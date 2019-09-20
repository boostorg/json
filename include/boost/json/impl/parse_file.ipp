//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_PARSE_FILE_IPP
#define BOOST_JSON_IMPL_PARSE_FILE_IPP

#include <boost/beast/core/file.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/detail/clamp.hpp>

namespace boost {
namespace beast {
namespace json {

void
parse_file(
    char const* path,
    basic_parser& parser,
    error_code& ec)
{
    file f;
    f.open(path, beast::file_mode::scan, ec);
    if(ec)
        return;
    flat_buffer b;
    auto remain = f.size(ec);
    if(ec)
        return;
    while(remain > 0)
    {
        auto amount = beast::detail::clamp(remain);
        auto mb = b.prepare(amount);
        b.commit(f.read(mb.data(), mb.size(), ec));
        if(ec)
            return;
        if(remain == b.size())
            break;
        auto bytes_used =
            parser.write_some(b.data(), ec);
        if(ec)
            return;
        remain -= b.size();
        b.consume(bytes_used);
    }
    parser.write(b.data(), ec);
    if(ec)
        return;
    // finished
}

} // json
} // beast
} // boost

#endif
