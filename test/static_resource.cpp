//
// Copyright (c) 2020 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

// Test that header file is self-contained.
#include <boost/json/static_resource.hpp>

#include <boost/json/parse.hpp>
#include <boost/json/to_string.hpp>
#include <iostream>

#include "test_suite.hpp"

BOOST_JSON_NS_BEGIN

class static_resource_test
{
public:
    void
    testJavadocs()
    {
    //--------------------------------------

    unsigned char buf[ 4000 ];
    static_resource mr( buf );

    // Parse the string, using our memory resource
    auto const jv = parse( "[1,2,3]", &mr );

    // Print the JSON
    std::cout << jv;

    //--------------------------------------
    }

    void
    test()
    {
        {
            char buf[1000];
            static_resource mr(
                buf, sizeof(buf));
            BOOST_TEST(to_string(parse(
                "[1,2,3]", &mr)) == "[1,2,3]");
        }
        {
            char buf[10];
            static_resource mr(
                buf, sizeof(buf));
            BOOST_TEST_THROWS(
                to_string(parse("[1,2,3]", &mr)),
                std::bad_alloc);
        }
    }

    void
    run()
    {
        test();
    }
};

TEST_SUITE(static_resource_test, "boost.json.static_resource");

BOOST_JSON_NS_END
