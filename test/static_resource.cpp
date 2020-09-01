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

#include "test_suite.hpp"

BOOST_JSON_NS_BEGIN

class static_resource_test
{
public:
    void
    test()
    {
        char buf[1000];
        static_resource mr(
            buf, sizeof(buf));
        BOOST_TEST(to_string(parse(
            "[1,2,3]", &mr)) == "[1,2,3]");
    }

    void
    run()
    {
        test();
    }
};

TEST_SUITE(static_resource_test, "boost.json.static_resource");

BOOST_JSON_NS_END
