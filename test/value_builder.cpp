//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

// Test that header file is self-contained.
#include <boost/json/value_builder.hpp>

#include <boost/json/monotonic_resource.hpp>
#include <boost/json/to_string.hpp>

#include "test_suite.hpp"

namespace boost {
namespace json {

class value_builder_test
{
public:
    // This is from the javadoc
    void
    testBuilder()
    {
    // This example builds a json::value without any dynamic memory allocations:

    // Construct the builder using a local buffer
    char temp[4096];
    value_builder vb( storage_ptr(), temp, sizeof(temp) );

    // Create a monotonic resource with a local initial buffer
    char buf[4096];
    monotonic_resource mr( buf, sizeof(buf) );

    // The builder will create a value using `mr`
    vb.reset(&mr);

    // Iteratively create the elements
    vb.begin_object();
    vb.insert_key("a");
    vb.insert_int64(1);
    vb.insert_key("b");
    vb.insert_null();
    vb.insert_key("c");
    vb.insert_string("hello");
    vb.end_object();

    // Take ownership of the value
    value jv = vb.release();

    assert( to_string(jv) == "{\"a\":1,\"b\":null,\"c\":\"hello\"}" );

    // At this point we could re-use the builder by calling reset

    }

    void
    run()
    {
        testBuilder();
    }
};

TEST_SUITE(value_builder_test, "boost.json.value_builder");

} // json
} // boost
