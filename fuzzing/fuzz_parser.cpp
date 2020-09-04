// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Paul Dreik (github@pauldreik.se)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#include <boost/json/parser.hpp>
#include <boost/json/parse_options.hpp>
using namespace boost::json;

bool
fuzz_parser(parse_options popt,string_view sv)
{

    parser p(storage_ptr{},popt);
    error_code ec;

    // This must be called once before parsing every new JSON
    p.reset();

    // Write the first part of the buffer
    p.write( sv.data(), sv.size(), ec);

    if(! ec)
        p.finish( ec );

    // Take ownership of the resulting value.
    if(! ec)
    {
        value jv = p.release( ec );
        if(! ec )
            return jv.is_number();
    }
    return false;
}

extern "C"
int
LLVMFuzzerTestOneInput(
        const uint8_t* data, size_t size)
{
    if(size<1)
        return 0;

    parse_options popt;
    popt.allow_comments=!!(data[0]&0x1);
    popt.allow_trailing_commas=!!(data[0]&0x2);
    popt.allow_invalid_utf8=!!(data[0]&0x4);
    popt.max_depth= (data[0]>>3);

    data+=1;
    size-=1;

    try
    {

        string_view view{
            reinterpret_cast<const char*>(
                        data), size};
        fuzz_parser(popt,view);
    }
    catch(...)
    {
    }
    return 0;
}

