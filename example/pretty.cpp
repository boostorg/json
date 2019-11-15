//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

/*
    This example parses a JSON file and pretty-prints
    it to standard output.
*/

#include <boost/json.hpp>
#include <cstdio>
#include <iostream>

namespace json = boost::json;

json::value
parse_file( char const* filename )
{
    json::error_code ec;
    auto f = std::fopen( filename, "r" );
    if( ! f )
    {
        ec.assign( errno, json::generic_category() );
        throw json::system_error(ec);
    }
    json::parser p;
    p.start();
    do
    {
        char buf[4096];

        // Read from the file into our buffer.
        auto const nread = fread( buf, 1, sizeof(buf), f );
        if( std::ferror(f) )
        {
            ec.assign( errno, json::generic_category() );
            throw json::system_error(ec);
        }

        auto nparsed = p.write_some( buf, nread, ec);

        // Make sure we use all the characters in the file.
        if( ! ec && nparsed < nread )
            nparsed = p.write_some( buf + nparsed, sizeof(buf) - nparsed, ec );

        if( ec )
            throw json::system_error(ec);
    }
    while( ! std::feof(f) );

    // Tell the parser there is no more serialized JSON.
    p.write_eof(ec);
    if( ec )
        throw json::system_error(ec);

    return p.release();
}

void
pretty_print( std::ostream& os, json::value const& jv )
{
}

int
main(int argc, char** argv)
{
    if(argc != 2)
    {
        std::cerr <<
            "Usage: pretty <filename>"
            << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        // Parse the file as JSON
        auto const jv = parse_file( argv[1] );

        // Now pretty-print the value
        pretty_print(std::cout, jv);
    }
    catch(std::exception const& e)
    {
        std::cerr <<
            "Caught exception: "
            << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
