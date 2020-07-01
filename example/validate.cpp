//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

/*
    This example verifies that a file contains valid JSON.
    It is implementing by subclassing basic_parser 
*/

#include <boost/json.hpp>

// This file must be manually included when
// using basic_parser to implement a parser.
#include <boost/json/basic_parser.hpp>

#include <iomanip>
#include <iostream>

#include "file.hpp"

using namespace boost::json;

//[example_validate

bool
validate( string_view s )
{
    // The null parser discards all the data

    class null_parser : public basic_parser
    {
        friend class boost::json::basic_parser;

    public:
        null_parser() {}
        ~null_parser() {}
        bool on_document_begin( error_code& ) { return true; }
        bool on_document_end( error_code& ) { return true; }
        bool on_object_begin( error_code& ) { return true; }
        bool on_object_end( error_code& ) { return true; }
        bool on_array_begin( error_code& ) { return true; }
        bool on_array_end( error_code& ) { return true; }
        bool on_key_part( string_view, error_code& ) { return true; }
        bool on_key( string_view, error_code& ) { return true; }
        bool on_string_part( string_view, error_code& ) { return true; }
        bool on_string( string_view, error_code& ) { return true; }
        bool on_int64( std::int64_t, error_code& ) { return true; }
        bool on_uint64( std::uint64_t, error_code& ) { return true; }
        bool on_double( double, error_code& ) { return true; }
        bool on_bool( bool, error_code& ) { return true; }
        bool on_null( error_code& ) { return true; }
        
        std::size_t
        write(
            char const* data,
            std::size_t size,
            error_code& ec)
        {
            auto const n =
                basic_parser::write_some(
                *this, false, data, size, ec);
            if(! ec && n < size)
                ec = error::extra_data;
            return n;
        }
    };

    // Parse with the null parser and return false on error
    null_parser p;
    error_code ec;
    p.write( s.data(), s.size(), ec );
    if( ec )
        return false;

    // The string is valid JSON.
    return true;
}

//]

int
main(int argc, char** argv)
{
    if(argc != 2)
    {
        std::cerr <<
            "Usage: validate <filename>"
            << std::endl;
        return EXIT_FAILURE;
    }

    try
    {
        // Read the file into a string
        auto const s = read_file( argv[1] );

        // See if the string is valid JSON
        auto const valid = validate( s );

        // Print the result
        if( valid )
            std::cout << argv[1] << " contains a valid JSON\n";
        else
            std::cout << argv[1] << " does not contain a valid JSON\n";
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
