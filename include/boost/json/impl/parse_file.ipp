//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_PARSE_FILE_IPP
#define BOOST_JSON_IMPL_PARSE_FILE_IPP

#include <boost/system/error_code.hpp>
#include <cstdio>

namespace boost {
namespace json {

void
parse_file(
    char const* path,
    basic_parser& parser,
    error_code& ec)
{
    auto const& gc =
        boost::system::generic_category();
    
    struct cleanup
    {
        FILE* f;
        ~cleanup()
        {
            ::fclose(f);
        }
    };

    auto f = ::fopen(path, "rb");
    if(! f)
    {
        ec = error_code(errno, gc);
        return;
    }
    cleanup c{f};
    std::size_t result;
    result = ::fseek(f, 0, SEEK_END);
    if(result != 0)
    {
        ec = error_code(errno, gc);
        return;
    }
    auto const size = ::ftell(f);
    if(size == -1L)
    {
        ec = error_code(errno, gc);
        return;
    }
    char* buf = new char[size];
    result = ::fread(buf, 1, size, f);
    if(result != 0)
    {
        ec = error_code(errno, gc);
        return;
    }
    parser.write(buf, size, ec);
    delete[] buf;
}

} // json
} // boost

#endif
