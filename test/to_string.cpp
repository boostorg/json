//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

// Test that header file is self-contained.
#include <boost/json/to_string.hpp>

#include <boost/json/parse.hpp>

#include "test_suite.hpp"

BOOST_JSON_NS_BEGIN

class to_string_test
{
public:
    void
    testToString()
    {
        BOOST_TEST(
            to_string(parse("[1,2,3]")) == "[1,2,3]");
    }

    void
    testOstream()
    {
    }

    void
    run()
    {
        testToString();
        testOstream();
    }
};

TEST_SUITE(to_string_test, "boost.json.to_string");

BOOST_JSON_NS_END
