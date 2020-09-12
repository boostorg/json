//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#include <boost/json.hpp>

#include "test_suite.hpp"

BOOST_JSON_NS_BEGIN

//[doc_allocators_1
void do_rpc( string_view s )
{
    // The parser will use this storage for its temporary needs
    unsigned char temp[ 4000 ];

    // The null resource guarantees we will never dynamically allocate
    null_resource mr1;

    // Construct a strict parser using the temp buffer and no dynamic memory
    parser p( &mr1, parse_options(), temp );

    // Now we need a buffer to hold the actual JSON values
    unsigned char buf[ 6000 ];

    // The static resource is monotonic, using only a caller-provided buffer
    static_resource mr2( buf );

    // We need to catch any exceptions thrown by the two memory resources
    try
    {
        // This error code indicates errors not related to memory exhaustion
        error_code ec;

        // Parse the entire string we received from the network client
        p.write( s, ec );

        // Inform the parser that the complete input has been provided
        if(! ec )
            p.finish( ec );

        if(! ec )
        {
            // Retrieve the value. It will use `buf` for storage.
            value jv = p.release();

            // At this point we can inspect jv and perform the requested RPC.
        }
        else
        {
            // An error occurred. A real program would report the error
            // message back to the network client, indicating that the
            // received JSON was invalid.
        }

    }
    catch(std::bad_alloc const&)
    {
        // The memory needed to parse this JSON exceeded our statically
        // define upper limits. A real program would send an error message
        // back to the network client informing that their JSON is too large.
    }
}
//]

//----------------------------------------------------------

class doc_allocators_test
{
public:
    void
    run()
    {
        BOOST_TEST_PASS();
    }
};

TEST_SUITE(doc_allocators_test, "boost.json.doc_allocators");

BOOST_JSON_NS_END
